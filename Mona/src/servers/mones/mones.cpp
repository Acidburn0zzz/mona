/*!
  \file  mones.cpp
  \brief Code:Monesメイン

  Copyright (c) 2004 Yamami
  All rights reserved.
  License=MIT/X License

  \author  Yamami
  \version $Revision$
  \date   create:2004/08/08 update:$Date$
*/


//ここで、Monesのグローバルオブジェクトをインスタンス化する
#define MONES_GLOBAL_VALUE_DEFINED
#include "MonesGlobal.h"

#include <monapi.h>
#include "AbstractMonic.h"
#include "MoEther.h"
#include "MoIcmp.h"
#include "MonesConfig.h"
#include "MonesLoader.h"



using namespace MonAPI;


/*!
    \brief MonaMain
        Code:Monesメイン

    \param List<char*>* pekoe

    \author  Yamami
    \date    create:2004/08/08 update:2004/10/31
*/
int MonaMain(List<char*>* pekoe)
{
    
    int ret;
    
    //MonesRList = new HList<MONES_IP_REGIST*>();
    //MONES_IP_REGIST *regist;
    
    //このプロセス動作中は、IOを特権レベル3まで許可する
    syscall_get_io();
    
    // NICのインスタンス化
    // MonesLoaderクラス経由でインスタンス化する。
    AbstractMonic* insAbstractNic;
    
    MonesLoader* insNicLoader = new MonesLoader();
    insNicLoader->setup();
    insAbstractNic = insNicLoader->getNicInstance();
    //NICのロードに失敗した場合は、Mones終了
    if(insAbstractNic == 0){
        printf("NIC Error Mones Quit \n");
        exit(1);
    }
    
    //Etherクラスのインスタンス化
    g_MoEther = new MoEther();
    g_MoEther->etherInit(insAbstractNic);

    //ARPクラスのインスタンス化
    g_MoArp = new MoArp();
    g_MoArp->initArp(insAbstractNic);

    //IPクラスのインスタンス化
    g_MoIp = new MoIp();
    g_MoIp->initIp(insAbstractNic);

    //UDPクラスのインスタンス化
    g_MoUdp = new MoUdp();
    g_MoUdp->initUdp(insAbstractNic);

    //TCPクラスのインスタンス化
    g_MoTcp = new MoTcp();
    g_MoTcp->initTcp(insAbstractNic);

    //SocketsManagerクラスのインスタンス化
    g_SocketsManager = new SocketsManager();
    
    // Server start ok
    MessageInfo info;
    dword targetID = Message::lookupMainThread("MONITOR.BIN");
    if (targetID == 0xFFFFFFFF)
    {
        printf("Mones:INIT not found\n");
        exit(1);
    }

    // create message
    Message::create(&info, MSG_SERVER_START_OK, 0, 0, 0, NULL);

    // send
    if (Message::send(targetID, &info)) {
        printf("Mones:INIT error\n");
    }

    //ネットワークのIRQマスクEnable
    insAbstractNic->enableNetWork();

    //IRQレシーバとして登録 (IRQは、NICドライバクラスより得る)
    syscall_set_irq_receiver(insAbstractNic->getNicIRQ());

    //Monesのプライオリティを高く
    syscall_change_base_priority(1);

    /* Message loop */
    //ここでメッセージループ
    for (;;)
    {
        /* receive */
        if (!Message::receive(&info))
        {
            
            switch(info.header)
            {
            
            //NICハードウェアからの割り込み要求
            case MSG_INTERRUPTED:
                
                //logprintf("MSG_INTERRUPTED\n");
                                
                //パケット受信し待ちリストへ登録
                insAbstractNic->frame_input_public();
                
                break;

            //Monesからのパケット処理要求
            //TODO 本来は、プロトコル層の仕事
            // ここのスコープを明示は重要 crosses initialization
            case MSG_MONES_FRAME_REQ: {
                
                //logprintf("MSG_MONES_FRAME_REQ\n");
                
                //待ちはあるか？
                if(insAbstractNic->waitFrameBufList->isEmpty()){
                    break;
                }
                
                //共有メモリから待ちパケット取得                
                WAIT_FRAME_BUF* bgetWork = insAbstractNic->waitFrameBufList->removeAt(0);
                
                monapi_cmemoryinfo* cmPac;

                cmPac = monapi_cmemoryinfo_new();
                cmPac->Handle = bgetWork->cmHandle;
                //cmPac->Owner  = tid;
                cmPac->Size   = bgetWork->cmSize;
                monapi_cmemoryinfo_map(cmPac);

                dword    i;
                //Etherクラスに登録
                i = g_MoEther->setEtherFrame(cmPac->Data ,cmPac->Size);
                //イーサネットフレーム受信処理
                i = g_MoEther->receiveEther();                
                
                //共有メモリ解放
                monapi_cmemoryinfo_dispose(cmPac);
                monapi_cmemoryinfo_delete(cmPac);
                
                //リストメモリ解放
                delete bgetWork;

                break;
            }

            //アプリからのMonesへ登録
            case MSG_MONES_REGIST:
                
                //printf("MSG_MONES_REGIST\n");
                
                //リスンリストに登録
                g_SocketsManager->registLisSocket(info.from , (word)info.arg2);
                
                break;

            //アプリからのパケット送信要求
            case MSG_MONES_IP_SEND:
                
                //printf("MSG_MONES_IP_SEND\n");
                
                //パケット送信
                //とりあえずは、MessageInfo のstr 128バイトまでサポート
                //それ以上のサイズは、共有メモリを用いる
                if(info.arg1 == 0){

                    int icmp_size;
                    icmp_size=info.length;

                    dword ip;
                    TRANS_BUF_INFO *tbi;
                    tbi = new TRANS_BUF_INFO();
                    
                    ICMP_HEADER *volatile icmpHead;
                    //icmpHead = new ICMP_HEADER();
                    icmpHead=(ICMP_HEADER*)info.str;
                    
                    
                    //送信先 引数2の値
                    ip = info.arg2;
                    
                    //ICMP チェックサム計算
                    icmpHead->chksum=0;
                    icmpHead->chksum=MoPacUtl::calcCheckSum((dword*)icmpHead,icmp_size);

                    //送信バッファテーブルの設定
                    tbi->data[2]=NULL;
                    tbi->size[2]=0;
                    tbi->data[1]=(char*)icmpHead;
                    tbi->size[1]=icmp_size;
                    tbi->ipType=IPPROTO_ICMP;
        
                    ret = g_MoIp->transIp(tbi , MoPacUtl::swapLong(ip) ,0, 0);
                    
                    //printf("MoPacUtl::swapLong(ip)=%x\n",MoPacUtl::swapLong(ip));
                }
                
                break;

            //ARP待ちからのWake Up
            case MSG_MONES_WAKEUP_ARP_WAIT:
                
                //printf("MSG_MONES_WAKEUP_ARP_WAIT\n");
                
                MAC_REPLY_WAIT* nowWait;
                
                //ARP要求待ちリストの検索
                for (int i = 0; i < g_MoArp->macWaitList->size() ; i++) {
                    nowWait = g_MoArp->macWaitList->get(i);
                    
                    if(nowWait->repFlg == 1){
                        //ARP解決済みなら、待ちパケットを送信する。
                        TRANS_BUF_INFO *tbi;
                        tbi = new TRANS_BUF_INFO();
                        
                        //送信バッファテーブルの設定
                        tbi->data[2]=NULL;
                        tbi->size[2]=0;
                        tbi->data[1]=(char*)MonAPI::MemoryMap::map(nowWait->ipPacketBuf01->Handle);
                        tbi->size[1]=nowWait->ipPacketBuf01->Size;
                        tbi->ipType=nowWait->ipType;
                        
                        ret = g_MoIp->transIp(tbi , nowWait->ip ,0, 0);
                        
                        
                        //待避していたIPパケットバッファの解放
                        //free(nowWait->ipPacketBuf);
                        monapi_cmemoryinfo_dispose(nowWait->ipPacketBuf01);
                        monapi_cmemoryinfo_delete(nowWait->ipPacketBuf01);
                        
                        
                        //待ちリストから削除
                        g_MoArp->macWaitList->removeAt(i);
                        //カウンタデクリメントがいる。
                        i--;
                    }
                }
                
                
                break;

            default:
                /* igonore this message */
                break;
            }

        }
    }
    return 0;
}

