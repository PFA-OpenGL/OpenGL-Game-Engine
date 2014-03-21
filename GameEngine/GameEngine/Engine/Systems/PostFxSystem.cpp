
#include	<Systems/PostFxSystem.hh>
#include	<Components/CameraComponent.hpp>

PostFxSystem::PostFxSystem(std::weak_ptr<AScene> scene) :
					System(scene),
					_cameraFilter(scene),
					_idealIllum(0.3f),
					_adaptationSpeed(0.15f),
					_maxDarkImprovement(1.0f),
					_maxLightDiminution(0),
					_curFactor(1.0f),
					_targetFactor(1.0f),
					_useHDR(true),
					_useBloom(true),
					_bloomTextureSize(0),
					_bloomSigma(3.0f),
					_bloomGlare(1.0f),
					_bloomMipmap(1)
{
}

PostFxSystem::~PostFxSystem()
{
	glDeleteTextures(1, &_bloomTexture);
}

void	PostFxSystem::initialize()
{
	_cameraFilter.requireComponent<Component::CameraComponent>();

	_scene.lock()->getInstance<Renderer>()->addShader("fboToScreenMultisampled", "../../Shaders/fboToScreen.vp", "../../Shaders/fboToScreenMultisampled.fp");
	_scene.lock()->getInstance<Renderer>()->addShader("fboToScreen", "../../Shaders/fboToScreen.vp", "../../Shaders/fboToScreen.fp");
	_quad.init(_scene);

	_modulateRender.init("../../ComputeShaders/HighDynamicRange.kernel");
	_bloom.init("../../ComputeShaders/Bloom.kernel");

	// Bloom texture
	glGenTextures(1, &_bloomTexture);
	glBindTexture(GL_TEXTURE_2D, _bloomTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void	PostFxSystem::mainUpdate(double time)
{
	for (auto c : _cameraFilter.getCollection())
	{
		std::shared_ptr<Component::CameraComponent>		camera = c->getComponent<Component::CameraComponent>();

		OpenGLTools::Framebuffer	&current = camera->frameBuffer.isMultisampled() ? camera->downSampling : camera->frameBuffer;

		current.bind();
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		// Compute HDR
		if (_useHDR)
			computeHdr(current);
		if (_useBloom)
			computeBloom(current);
	}
}

void		PostFxSystem::computeHdr(OpenGLTools::Framebuffer &camFbo)
{
	GLuint	colorTexture = camFbo.getTextureAttachment(GL_COLOR_ATTACHMENT0);
	// ----------------------------------------------------
	// HDR Pass
	// ----------------------------------------------------
	// Average colors:
	// ----------------------------------------------------
	glm::vec4	avgColor(0);

	glBindTexture(GL_TEXTURE_2D, colorTexture);

	glGenerateMipmap(GL_TEXTURE_2D);

	float	maxDimension = glm::max(static_cast<float>(camFbo.getSize().x), static_cast<float>(camFbo.getSize().y));
	int		mipMapNbr = static_cast<int>(glm::floor(glm::log2(maxDimension)));

	glGetTexImage(GL_TEXTURE_2D, mipMapNbr, GL_RGBA, GL_FLOAT, &avgColor);
	// ----------------------------------------------------
	// Modulate colors:
	// ----------------------------------------------------
	size_t		WORK_GROUP_SIZE = 16;
	glm::uvec2	groupNbr = glm::uvec2((camFbo.getSize().x + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE,
		(camFbo.getSize().y + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE);

	float	avgIllumination = avgColor.r * 0.2126f + avgColor.g * 0.7152f + avgColor.b * 0.0722f;

	_targetFactor = glm::clamp(_idealIllum / avgIllumination, _maxLightDiminution, _maxDarkImprovement);

	_modulateRender.use();

	GLint		avgIllumLocation = glGetUniformLocation(_modulateRender.getId(), "factor");

	if (_curFactor != _targetFactor)
	{
		if (_curFactor < _targetFactor)
		{
			_curFactor += _adaptationSpeed * _scene.lock()->getInstance<Timer>()->getElapsed();
			if (_curFactor > _targetFactor)
				_curFactor = _targetFactor;
		}
		else
		{
			_curFactor -= _adaptationSpeed * _scene.lock()->getInstance<Timer>()->getElapsed();
			if (_curFactor < _targetFactor)
				_curFactor = _targetFactor;
		}
	}

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// Set the uniform
	glUniform1f(avgIllumLocation, _curFactor);

	// Bind color texture to modulate
	glBindImageTexture(0, colorTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	glDispatchCompute(groupNbr.x, groupNbr.y, 1);
}

void		PostFxSystem::computeBloom(OpenGLTools::Framebuffer &camFbo)
{
	GLuint	colorTexture = camFbo.getTextureAttachment(GL_COLOR_ATTACHMENT0);

	size_t		WORK_GROUP_SIZE = 16;
	glm::uvec2	groupNbr = glm::uvec2((camFbo.getSize().x + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE,
		(camFbo.getSize().y + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE);
	// ----------------------------------------------------
	// Bloom Pass:
	// ----------------------------------------------------
	// Creating bloom texture
	// ----------------------------------------------------

	glm::uvec2		mipmapDim;

	mipmapDim.x = static_cast<uint32_t>(glm::max(1.0f, glm::floor(camFbo.getSize().x / glm::pow(2.0f, static_cast<float>(_bloomMipmap)))));
	mipmapDim.y = static_cast<uint32_t>(glm::max(1.0f, glm::floor(camFbo.getSize().y / glm::pow(2.0f, static_cast<float>(_bloomMipmap)))));

	if (_bloomTextureSize != mipmapDim)
	{
		glBindTexture(GL_TEXTURE_2D, _bloomTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, camFbo.getSize().x, camFbo.getSize().y, 0, GL_RGBA, GL_FLOAT, NULL);
		_bloomTextureSize = mipmapDim;
	}

	// ----------------------------------------------------
	_bloom.use();

	GLint		sigmaLocation = glGetUniformLocation(_bloom.getId(), "sigma");
	GLint		passLocation = glGetUniformLocation(_bloom.getId(), "pass");
	GLint		glareLocation = glGetUniformLocation(_bloom.getId(), "glareFactor");

	glUniform1f(sigmaLocation, _bloomSigma);
	glUniform1f(glareLocation, _bloomGlare);
	glUniform2i(passLocation, 1, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);

	//		glGenerateMipmap(GL_TEXTURE_2D);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, _bloomMipmap);

	glBindImageTexture(1, _bloomTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glDispatchCompute(groupNbr.x, groupNbr.y, 1);

	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

	glUniform2i(passLocation, 0, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _bloomTexture);
	glBindImageTexture(1, colorTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glDispatchCompute(groupNbr.x, groupNbr.y, 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}