#include <string>
#include <fstream>
#include "MacroFilterTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MacroFilterTest);

using namespace monash;
using namespace std;

void MacroFilterTest::setUp()
{
}

void MacroFilterTest::tearDown()
{
}

void MacroFilterTest::testFindDefineSyntaxes()
{
    char buf[1024];
    MacroFilter f;
    YAML yaml;
    loadYAML("find_define_syntaxes.yml", yaml);

    for (YAML::iterator it = yaml.begin(); it != yaml.end(); ++it)
    {
        strings* s = (*it);
        if (s->size() < 2)
        {
            fprintf(stderr, "bad yaml!\n");
        }

        string defineSyntax = s->at(0);
        Nodes defineSyntaxes;

        // check count of define-syntax
        f.findDefineSyntaxes(Node::fromString(defineSyntax), defineSyntaxes);
        sprintf(buf, "In %s %d define-syntax found, but should be %d\n", defineSyntax.c_str(), defineSyntaxes.size(), s->size() - 1);
        CPPUNIT_ASSERT_MESSAGE(buf, defineSyntaxes.size() == s->size() - 1);

        // check define-syntax exactly same?
        for (Nodes::size_type i = 0; i < defineSyntaxes.size(); i++)
        {
            Node* d        = defineSyntaxes[i];
            Node* expected = Node::fromString(s->at(i + 1));
            sprintf(buf, "%s unmatch\n %s\n", d->toString().c_str(), expected->toString().c_str());
            CPPUNIT_ASSERT_MESSAGE(buf, d->equals(expected));
        }
    }
}

void MacroFilterTest::testFilter()
{
    char buf[1024];
    YAML yaml;
    loadYAML("filter.yml", yaml);

    for (YAML::iterator it = yaml.begin(); it != yaml.end(); ++it)
    {
        strings* s = (*it);
        if (s->size() != 3)
        {
            fprintf(stderr, "bad yaml!\n");
        }

        string macro     = s->at(0).c_str();
        string macroCall = s->at(1).c_str();
        string expected  = s->at(2).c_str();

        MacroFilter f;
        f.findAndStoreDefineSyntaxes(Node::fromString(macro));
        Node* expectedNode = Node::fromString(expected);
        Node* macroCallNode = Node::fromString(macroCall);
        f.filter(macroCallNode);
        sprintf(buf, "[result]\n%s unmatch\n [expected]\n%s\n", macroCallNode->toString().c_str(), expectedNode->toString().c_str());
        CPPUNIT_ASSERT_MESSAGE(buf, macroCallNode->equals(expectedNode));
    }
}