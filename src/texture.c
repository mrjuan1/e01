#include "texture.h"

#include <malloc.h>

#include "config.h"
#include "file.h"
#include "log.h"
#include "mem.h"

#if 1
#include <GLES2/gl2ext.h>
#endif // 1

GLint textureMaxAnisotropy = 0;

void textureSystemInit() {
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &textureMaxAnisotropy);
	if(!textureMaxAnisotropy) {
		warn("Anisotropic texture filtering not supported.\n", 0);
		return;
	}

	unsigned char anisotropy = configGetAnisotropy();

	if(anisotropy > textureMaxAnisotropy) {
		warn(
			"Config anisotropy (%u) higher than maximum supported anisotropy (%i), updating config...\n",
			anisotropy, textureMaxAnisotropy);

		anisotropy = (unsigned char)textureMaxAnisotropy;
		configSetAnisotropy(anisotropy);
		configSave();
	}

	// debug("Texture anisotropy: %u/%i\n", anisotropy, textureMaxAnisotropy);
}

bool textureInit(GLuint *texture, const char *filename) {
	file *f = fileInit(filename, fm_read);
	if(!f) return false;

	unsigned char levels;
	fileSeek(f, f->size - sizeof(unsigned char));
	bool result = fileRead(f, "texture levels", &levels, sizeof(unsigned char));
	if(!result) {
		fileFree(f);
		return false;
	}

	if(!*texture) glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	fileSeek(f, 0);

	for(unsigned char i = 0; i < levels; i++) {
		unsigned short width;
		result =
			fileRead(f, "texture level width", &width, sizeof(unsigned short));
		if(!result) {
			fileFree(f);
			return false;
		}

		unsigned short height;
		result = fileRead(f, "texture level height", &height,
						  sizeof(unsigned short));
		if(!result) {
			fileFree(f);
			return false;
		}

		int size;
		result = fileRead(f, "texture level size", &size, sizeof(int));
		if(!result) {
			fileFree(f);
			return false;
		}

		char *data = memAlloc("texture level data", size);
		if(!data) {
			fileFree(f);
			return false;
		}

		result = fileRead(f, "texture level data", data, size);
		if(!result) {
			free(data);
			fileFree(f);

			return false;
		}

		glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGB8_ETC2, width,
							   height, 0, size, data);
		free(data);
	}

	fileFree(f);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(textureMaxAnisotropy)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
						(GLint)configGetAnisotropy());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}
