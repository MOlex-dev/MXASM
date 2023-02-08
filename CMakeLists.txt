cmake_minimum_required(VERSION 3.23)
project(mxasm)

set(CMAKE_CXX_STANDARD 23)

add_executable(mxasm src/mxasm.cpp include/lexer.hpp src/lexer.cpp include/lexer_token.hpp src/lexer_token.cpp include/util.hpp src/util.cpp include/parser.hpp include/parser_token.hpp src/parser.cpp src/parser_token.cpp include/serializer.hpp src/serializer.cpp include/serializable_token.hpp src/serializable_token.cpp include/exceptions/mxasm_exception.hpp src/exceptions/mxasm_exception.cpp include/exceptions/arguments_exception.hpp src/exceptions/arguments_excpetion.cpp include/exceptions/lexer_exception.hpp src/exceptions/lexer_exception.cpp include/exceptions/parser_excpetion.hpp src/exceptions/parser_exception.cpp)
