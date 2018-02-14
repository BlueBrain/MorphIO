//#include "lex.cpp"
//#include "parse.cpp"
#include "parse_ll.cpp"

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#if 0
int lex_main(int argc, const char *argv[]) {
  if(argc < 1) {
    std::cout << "Usage: " << argv[0] << " morph.asc\n";
  }

  lexertl::rules rules;
  lexertl::state_machine sm;
  build_lexer(rules, sm);

  std::ifstream ifs(argv[1]);
  std::string input((std::istreambuf_iterator<char>(ifs)),
                    (std::istreambuf_iterator<char>()));

  lexertl::siterator iter(input.begin(), input.end(), sm);
  lexertl::siterator end;

  for (; iter != end; ++iter) {
    if (iter->id == +Token::WS || iter->id == +Token::COMMENT)
      continue;
    std::cout << "Id: " << iter->id << ", Token: '" << iter->str() << "'\n";
  }

  return 0;
}
#endif

int main(int argc, const char *argv[])
{
    (void)argc;
    std::cout << "Start..." << argv[1] << '\n';

    minimorph::NeurolucidaParser np;

    std::ifstream ifs(argv[1]);
    std::string input((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
    bool ret = np.parse(input);

    if (!ret)
    {
        std::cout << "parse failed" << argv[1] << '\n';
    }

    return 0;
}
