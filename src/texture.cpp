#include <texture.hpp>
#include <stb_image.h>

Texture::Texture(std::string filename) {

    this->path = filename;
    this->name = stripPath(filename);

}

void Texture::load(std::string filename) {

    this->path = filename;
    this->name = stripPath(filename);

}

void Texture::generate(bool clamp) {

    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(this->path.c_str(), &width, &height, &nrChannels, 0);
    if(!data) {
        std::cout << "Failed to load texture " << this->path << std::endl;
    }

    switch(nrChannels) {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;
        default:
            std::cout << "Invalid number of channels for " << this->path << std::endl;
    } 
    
    glGenerateMipmap(GL_TEXTURE_2D);

    // set the texture wrapping/filtering options (on currently bound texture)
    if(clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    this->_isGenerated = GL_TRUE;

}

void Texture::bind(u32 location) {

    if(_isGenerated == GL_TRUE) {

        glActiveTexture(GL_TEXTURE0 + location);
        glBindTexture(GL_TEXTURE_2D, this->ID);

    }

}