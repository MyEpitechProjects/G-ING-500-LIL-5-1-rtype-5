/*
** EPITECH PROJECT, 2026
** AModule
** File description:
** AModule def
*/

#ifndef AMODULE
    #define AMODULE

    #include "IModule.hpp"
    #include <string>

class AModule : public IModule {
    protected:
        std::string _name;

    public:
        AModule(const std::string &name) : _name(name) {}
        virtual ~AModule() = default;

        AModule(const AModule &) = delete;
        AModule &operator=(const AModule &) = delete;
        AModule(AModule &&) = default;
        AModule &operator=(AModule &&) = default;

        const std::string &getName() const override { return _name; }
};

#endif
