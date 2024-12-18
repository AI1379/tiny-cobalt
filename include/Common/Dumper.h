//
// Created by Renatus Madrigal on 12/18/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_DUMPER_H_
#define TINY_COBALT_INCLUDE_COMMON_DUMPER_H_

#include <iostream>

namespace TinyCobalt::Utility {

    class Dumper {
    public:
        enum class Controller { PushIntend, PopIntend, Intend, EndLine, EndBlock, StartBlock };

        Dumper() : os(std::cout) {}
        Dumper(std::ostream &os) : os(os) {}
        Dumper(std::ostream &os, std::size_t intend) : os(os), intend(intend) {}

        Dumper &setIntend(std::size_t intend) {
            this->intend = intend;
            return *this;
        }

        Dumper &setIntendChar(char intendChar) {
            this->intendChar = intendChar;
            return *this;
        }

        Dumper &setEndLineChar(char endLineChar) {
            this->endLineChar = endLineChar;
            return *this;
        }
        Dumper &setBlockChar(char startBlockChar, char endBlockChar) {
            this->startBlockChar = startBlockChar;
            this->endBlockChar = endBlockChar;
            return *this;
        }

        template<typename T>
        Dumper &operator<<(const T &value) {
            os << value;
            return *this;
        }

        template<typename T>
        Dumper &dump(const T &value) {
            value.dump(*this);
            return *this;
        }

        Dumper &dump(const char *value) { return operator<<(value); }

        Dumper &endl() { return dump(Controller::EndLine); }

        Dumper &operator<<(Controller controller) {
            switch (controller) {
                case Controller::PushIntend:
                    currentIntend += intend;
                    break;
                case Controller::PopIntend:
                    currentIntend -= intend;
                    break;
                case Controller::Intend:
                    printIntend();
                    break;
                case Controller::EndLine:
                    os << endLineChar;
                    printIntend();
                    break;
                case Controller::EndBlock:
                    currentIntend -= intend;
                    os << endLineChar;
                    printIntend();
                    os << endBlockChar;
                    break;
                case Controller::StartBlock:
                    os << startBlockChar;
                    currentIntend += intend;
                    break;
            }
            return *this;
        }

        Dumper &dump(Controller controller) { return operator<<(controller); }


    private:
        void printIntend() {
            for (std::size_t i = 0; i < currentIntend; ++i) {
                os << intendChar;
            }
        }

        std::ostream &os;
        std::size_t intend = 0;
        char intendChar = ' ';
        std::size_t currentIntend = 0;
        char endLineChar = '\n';
        char startBlockChar = '{';
        char endBlockChar = '}';
    };

} // namespace TinyCobalt::Utility

#endif // TINY_COBALT_INCLUDE_COMMON_DUMPER_H_