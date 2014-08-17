#include <Render/RenderPass.hh>
#include <cassert>
#include <string>
#include <Core/Drawable.hh>
#include <Render/OpenGLTask.hh>
#include <Render/GeometryManager.hh>
#include <Render/Storage.hh>

# undef DEBUG_MESSAGE
# define DEBUG_MESSAGE(type, from, reason, return_type) \
	{	assert(0 && std::string(std::string(type) + ": from[" + std::string(from) + "], reason[" + std::string(reason) + "].").c_str()); return return_type; }


namespace gl
{

	Render::~Render()
	{
		for (size_t index = 0; index < _tasks.size(); ++index)
		{	
			for (uint8_t param = 0; param < _tasks[index].nbrParams; ++param)
				delete _tasks[index].params[param];
			delete[] _tasks[index].params;
		}
	}

	Render &Render::pushSetScissorTask(glm::ivec4 const &area)
	{
		Task task;

		setTaskAllocation(task, area);
		task.func = setScissor;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetClearValueTask(glm::vec4 const &color, float depth, uint8_t stencil)
	{
		Task task;

		setTaskAllocation(task, color, depth, stencil);
		task.func = setClearValue;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetColorMaskTask(GLuint index, glm::bvec4 const &color)
	{
		Task task;

		setTaskAllocation(task, index, color);
		task.func = setColorMask;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetDepthMaskTask(bool depth)
	{
		Task task;

		setTaskAllocation(task, depth);
		task.func = setDepthMask;
		_tasks.push_back(task);
		return (*this);	return (*this);
	}

	Render &Render::pushSetStencilMaskTask(uint8_t front, uint8_t back)
	{
		Task task;

		setTaskAllocation(task, front, back);
		task.func = setStencilMask;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushClearTask(bool color, bool depth, bool stencil)
	{
		Task task;

		setTaskAllocation(task, color, depth, stencil);
		task.func = clear;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetStencilFunctionFrontFaceTask(GLenum func, int ref, uint8_t mask)
	{
		Task task;

		setTaskAllocation(task, func, ref, mask);
		task.func = setStencilFunctionFrontFace;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetStencilOperationFrontFaceTask(GLenum opStencilFail, GLenum opDepthFail, GLenum opDepthPass)
	{
		Task task;

		setTaskAllocation(task, opStencilFail, opDepthFail, opDepthPass);
		task.func = setStencilOperationFrontFace;
		_tasks.push_back(task);
		return (*this);
	}
	
	Render &Render::pushSetStencilFunctionBackFaceTask(GLenum func, int ref, uint8_t mask)
	{
		Task task;
		setTaskAllocation(task, func, ref, mask);
		task.func = setStencilFunctionBackFace;
		_tasks.push_back(task);
		return (*this);
	}
	
	Render &Render::pushSetStencilOperationBackFaceTask(GLenum opStencilFail, GLenum opDepthFail, GLenum opDepthPass)
	{
		Task task;

		setTaskAllocation(task, opStencilFail, opDepthFail, opDepthPass);
		task.func = setStencilOperationBackFace;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetStencilFunctionTask(GLenum func, int ref, uint8_t mask)
	{
		Task task;

		setTaskAllocation(task, func, ref, mask);
		task.func = setStencilFunction;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetStencilOperationTask(GLenum opStencilFail, GLenum opDepthFail, GLenum opDepthPass)
	{
		Task task;

		setTaskAllocation(task, opStencilFail, opDepthFail, opDepthPass);
		task.func = setStencilOperation;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetBlendTask(int drawBuffer, bool state)
	{
		Task task;

		setTaskAllocation(task, drawBuffer, state);
		task.func = setBlend;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetBlendEquationTask(GLenum colorMode, GLenum alphaMode)
	{
		Task task;

		setTaskAllocation(task, colorMode, alphaMode);
		task.func = setBlendEquationSeparate;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetBlendEquationTask(GLenum mode)
	{
		Task task;

		setTaskAllocation(task, mode);
		task.func = setBlendEquation;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetBlendFuncTask(GLenum srcRGB, GLenum destRGB, GLenum srcAlpha, GLenum destAlpha)
	{
		Task task;

		setTaskAllocation(task, srcRGB, destRGB, srcAlpha, destAlpha);
		task.func = setBlendFuncSeparate;
		_tasks.push_back(task);
		return (*this);
	}
	Render &Render::pushSetBlendFuncTask(GLenum src, GLenum dest)
	{
		Task task;

		setTaskAllocation(task, src, dest);
		task.func = setBlendFunc;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetBlendConstantTask(glm::vec4 const &blendColor)
	{
		Task task;

		setTaskAllocation(task, blendColor);
		task.func = setBlendConstant;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::pushSetTestTask(bool scissor, bool stencil, bool depth)
	{
		Task task;

		setTaskAllocation(task, scissor, stencil, depth);
		task.func = setTest;
		_tasks.push_back(task);
		return (*this);
	}

	Render &Render::popTask()
	{
		if (!(_tasks.size() > 0))
			DEBUG_MESSAGE("Warning", "RenderPass - popTask", "No task to pop", *this);
		auto &element = _tasks.back();
		for (uint8_t index = 0; index < element.nbrParams; ++index)
			delete element.params[index];
		delete[] element.params;
		_tasks.pop_back();
		return (*this);
	}

	Render &Render::configRect(glm::ivec4 const &rect)
	{
		_rect = rect;
		return (*this);
	}

	Render &Render::setMode(GLenum mode)
	{
		_mode = mode;
		return (*this);
	}

	GLenum Render::getMode() const
	{
		return (_mode);
	}

	Render &Render::branchInput(RenderOffScreen const &input)
	{
		_branch = &input;
		_updateInput = true;
		return (*this);
	}

	Render &Render::unBranchInput()
	{
		_updateInput = false;
		_branch = NULL;
		return (*this);
	}

	Render &Render::pushInputSampler(Key<Sampler> const &key)
	{
		if (_shader.hasSampler(key))
			_inputSamplers.push_back(key);
		else
			DEBUG_MESSAGE("Warning", "pushInputSampler", "input not present into this shader attach", *this);
		return (*this);
	}

	Render &Render::popInputSampler()
	{
		_inputSamplers.pop_back();
		return (*this);
	}
	
	RenderOffScreen::RenderOffScreen(Shader &shader)
		: Render(shader),
		_sample(1),
		_colorAttachement(NULL),
		_colorTexture2D(NULL),
		_nbrColorAttachement(0),
		_updateOutput(false)
	{
	}

	RenderOffScreen::~RenderOffScreen()
	{
		if (_nbrColorAttachement > 0)
		{
			delete[] _colorAttachement;
			delete[] _colorTexture2D;
		}
	}

	RenderOffScreen &RenderOffScreen::configSample(GLint sample)
	{
		_sample = sample;
		return (*this);
	}

	RenderOffScreen &RenderOffScreen::pushColorOutput(GLenum attachement, GLenum internalFormat)
	{
		return (pushColorOutput(attachement, _rect.z, _rect.w, internalFormat));
	}

	RenderOffScreen &RenderOffScreen::pushColorOutput(GLenum attachement, size_t width, size_t height, GLenum internalFormat)
	{
		GLsizei tmp_nbrColorAttachement = _nbrColorAttachement + 1;
		GLenum *tmp_colorAttachement = new GLenum[tmp_nbrColorAttachement];
		Texture2D **tmp_colorTexture2D = new Texture2D *[tmp_nbrColorAttachement];
		memcpy(tmp_colorAttachement, _colorAttachement, sizeof(GLenum)* _nbrColorAttachement);
		memcpy(tmp_colorTexture2D, _colorTexture2D, sizeof(GLenum)* _nbrColorAttachement);
		if (_nbrColorAttachement > 0)
		{
			delete[] _colorAttachement;
			delete[] _colorTexture2D;
		}
		_colorAttachement = tmp_colorAttachement;
		_colorTexture2D = tmp_colorTexture2D;
		_nbrColorAttachement = tmp_nbrColorAttachement;
		_colorTexture2D[_nbrColorAttachement - 1] = new Texture2D(width, height, internalFormat, false);
		_colorAttachement[_nbrColorAttachement - 1] = attachement;
		_updateOutput = true;
		return (*this);
	}

	RenderOffScreen &RenderOffScreen::popColorOutput()
	{
		if (_nbrColorAttachement < 1)
			return (*this);
		GLsizei tmp_nbrColorAttachement = _nbrColorAttachement - 1;
		GLenum *tmp_colorAttachement = new GLenum[tmp_nbrColorAttachement];
		Texture2D **tmp_colorTexture2D = new Texture2D *[tmp_nbrColorAttachement];
		memcpy(tmp_colorAttachement, _colorAttachement, sizeof(GLenum)* tmp_nbrColorAttachement);
		memcpy(tmp_colorTexture2D, _colorTexture2D, sizeof(GLenum)* tmp_nbrColorAttachement);
		delete[] _colorAttachement;
		delete[] _colorTexture2D;
		_colorAttachement = tmp_colorAttachement;
		_colorTexture2D = tmp_colorTexture2D;
		_updateOutput = true;
		return (*this);
	}

	Texture2D const &RenderOffScreen::getColorOutput(size_t index) const
	{
		return (*_colorTexture2D[index]);
	}

	GLenum RenderOffScreen::getAttachementOutput(size_t index)
	{
		return (_colorAttachement[index]);
	}

	RenderPass::RenderPass(Shader &shader)
		: RenderOffScreen(shader),
		_objectsToRender(NULL)
	{
	}

	RenderPass::~RenderPass()
	{
	}

	RenderPass &RenderPass::setRenderPassObjects(AGE::Vector<AGE::Drawable> const &objects)
	{
		_objectsToRender = &objects;
		return (*this);
	}

	Render &RenderPass::draw()
	{
		_fbo.bind();
		if (_updateOutput)
			updateOutput();
		if (_inputSamplers.size() == _nbrColorAttachement && _branch != NULL)
		{
			for (size_t index = 0; index < _inputSamplers.size(); ++index)
				_shader.setSampler(_inputSamplers[index], _branch->getColorOutput(index));
		}
		return (*this);
	}

	//Render &RenderPass::draw()
	//{
	//	if (_shader == NULL)
	//		DEBUG_MESSAGE("Warning", "RenderPass - update", "shader bind to renderPass assign to NULL", *this);
	//	_fbo.bind();
	//	_fbo.size(_rect.z, _rect.w, _sample);
	//	glViewport(_rect.x, _rect.y, _rect.z, _rect.w);
	//	for (size_t index = 0; index < _tasks.size(); ++index)
	//		_tasks[index].func(_tasks[index].params);
	//	updateOutput();
	//	updateInput();
	//	return (*this);
	//}


	//Texture2D const &Render::getColorOutput(size_t index) const
	//{
	//	return (*_colorOutput[index].second);
	//}

	//size_t Render::getNbrColorOuput() const
	//{
	//	return (_colorOutput.size());
	//}

	//void RenderPass::updateOutput()
	//{
	//	// update output
	//	if (_updateColorOutput)
	//	{
	//		GLenum *targeted;
	//		targeted = new GLenum[_colorOutput.size()];
	//		for (size_t index = 0; index < _colorOutput.size(); ++index)
	//		{
	//			targeted[index] = _colorOutput[index].first;
	//			_colorOutput[index].second->attachement(_fbo, targeted[index]);
	//		}
	//		glDrawBuffers(GLsizei(_colorOutput.size()), targeted);
	//		_updateColorOutput = true;
	//	}
	//}
}