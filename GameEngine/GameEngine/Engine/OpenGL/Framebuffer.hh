#ifndef  __FRAMEBUFFER_HH__
# define __FRAMEBUFFER_HH__

#include <map>
#include <vector>
#include "Utils/OpenGL.hh"
#include "VertexBuffer.hh"

namespace OpenGLTools
{
	class Framebuffer
	{
	public:
	private:
		bool _isBinded;
		unsigned int _width;
		unsigned int _height;
		GLuint _handle;
		GLuint _depth;
		OpenGLTools::VertexBuffer _vbo;
		std::map<unsigned int, GLuint> _layers;
	public:
		Framebuffer();
		~Framebuffer();
		bool init(unsigned int width, unsigned int height);
		void bind();
		void unbind();
		void addLayer(unsigned int id);
		void bindTexture(unsigned int id);
		void renderToScreen();
		inline bool isBinded(){return _isBinded;};
		unsigned int bindTextures(const std::vector<unsigned int> &list);
		void unbindTextures(const std::vector<unsigned int> &list);
		void clear();
	private:
		Framebuffer(const Framebuffer &o);
		Framebuffer &operator=(const Framebuffer &o);
	};
}

#endif   //!__FRAMEBUFFER_HH__