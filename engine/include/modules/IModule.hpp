/*
** EPITECH PROJECT, 2026
** IModule
** File description:
** IModule def
*/

#ifndef IMODULE
    #define IMODULE

    #include "Registry.hpp"
    #include <string>

class IModule {
    public:
        virtual ~IModule() = default;

        virtual const std::string &getName() const = 0;
        virtual void registerComponents(Registry &registry) = 0;
        virtual void update(Registry &registry, double elapsedSeconds) = 0;
};

#endif
