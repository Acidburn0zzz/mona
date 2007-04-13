/*!
  \file   main.cpp
  \brief  scheme

  Copyright (c) 2006 Higepon
  WITHOUT ANY WARRANTY

  \author  Higepon
  \version $Revision$
  \date   create:2006/06/16 update:$Date$
*/
#include "scheme.h"

using namespace util;
using namespace monash;

int main(int argc, char *argv[])
{
    // if continuation failed, see cont_initialize function and fix cont_stack_bottom!
    scheme_init();

    if (argc == 1)
    {
        mona_shell_init();
        scheme_interactive();
        return 0;
    }
    else
    {
        scheme_exec_file(argv[1]);
        return 0;
    }
}
