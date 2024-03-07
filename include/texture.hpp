#pragma once

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <typedef.hpp>
#include <utils.hpp>

#define TEXTURE_NULL 0xffffffff

class Texture {

    private:
        u32 ID = TEXTURE_NULL;
        std::string path;
        std::string name;
        u32 _isGenerated = GL_FALSE;

    public:
        Texture(){};
        Texture(std::string filename);
        ~Texture(){};

        void load(std::string filename);
        void generate(bool clamp = false);
        void bind(u32 location);

        u32 getID() {return ID;};
        std::string getName() {return name;};
        u32 isGenerated() {return _isGenerated;};
    
};