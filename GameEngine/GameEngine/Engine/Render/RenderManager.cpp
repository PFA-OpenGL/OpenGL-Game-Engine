#include <Render/RenderManager.hh>
#include <Render/Storage.hh>
#include <Render/Pipeline.hh>
#include <algorithm>
#include <iostream>
#include <string>

namespace gl
{

	RenderManager::RenderManager()
		: _preShaderQuad(NULL)
	{
	}

	RenderManager::~RenderManager()
	{
		if (_preShaderQuad == NULL)
			delete _preShaderQuad;
		for (auto it = _shaders.begin(); it != _shaders.end(); ++it)
			delete it->second;
		for (auto it = _textures.begin(); it != _textures.end(); ++it)
			delete it->second;
		for (auto it = _renderPass.begin(); it != _renderPass.end(); ++it)
			delete it->second;
		for (auto it = _renderPostEffect.begin(); it != _renderPostEffect.end(); ++it)
			delete it->second;
	}

	RenderManager &RenderManager::createPreShaderQuad()
	{
		if (_preShaderQuad != NULL)
			return (*this);
		if ((_preShaderQuad = Shader::createPreShaderQuad()) == NULL)
			assert(0);
		_preShaderQuad->addSampler("input_sampler");
		return (*this);
	}

	Key<Shader> RenderManager::addComputeShader(std::string const &compute)
	{
		Key<Shader> key;
		Shader *shader;

		if ((shader = Shader::createComputeShader(compute)) == NULL)
			assert(0);
		_shaders[key] = shader;
		return (key);
	}

	Key<Shader> RenderManager::addShader(std::string const &vertex, std::string const &frag)
	{
		Key<Shader> key;
		Shader *shader;

		if ((shader = Shader::createShader(vertex, frag)) == NULL)
			assert(0);
		_shaders[key] = shader;
		return (key);
	}

	Key<Shader> RenderManager::addShader(std::string const &geo, std::string const &vertex, std::string const &frag)
	{
		Key<Shader> key;
		Shader *shader;

		if ((shader = Shader::createShader(vertex, frag, geo)) == NULL)
			assert(0);
		_shaders[key] = shader;
		return (key);
	}

	Key<Shader> RenderManager::getShader(size_t target) const
	{
		if (target >= _shaders.size())
			assert(0);
		auto &element = _shaders.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	Key<Uniform> RenderManager::getShaderUniform(Key<Shader> const &key, size_t target)
	{
		Shader const *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->getUniform(target));
	}

	Key<Uniform> RenderManager::addShaderUniform(Key<Shader> const &key, std::string const &flag)
	{
		Shader *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->addUniform(flag));
	}

	Key<Uniform> RenderManager::addShaderUniform(Key<Shader> const &key, std::string const &flag, glm::mat4 const &value)
	{
		Shader *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->addUniform(flag, value));
	}

	Key<Uniform> RenderManager::addShaderUniform(Key<Shader> const &key, std::string const &flag, glm::mat3 const &value)
	{
		Shader *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->addUniform(flag, value));
	}

	Key<Uniform> RenderManager::addShaderUniform(Key<Shader> const &key, std::string const &flag, glm::vec4 const &value)
	{
		Shader *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->addUniform(flag, value));
	}

	Key<Uniform> RenderManager::addShaderUniform(Key<Shader> const &key, std::string const &flag, float value)
	{
		Shader *shader;
		if ((shader = getShader(key)) == NULL)
			return (Key<Uniform>(KEY_DESTROY));
		return (shader->addUniform(flag, value));
	}

	RenderManager &RenderManager::setShaderUniform(Key<Shader> const &keyShader, Key<Uniform> const &key, glm::mat4 const &mat4)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (*this);
		shader->setUniform(key, mat4);
		return (*this);
	}

	RenderManager &RenderManager::setShaderUniform(Key<Shader> const &keyShader, Key<Uniform> const &key, glm::vec4 const &vec4)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (*this);
		shader->setUniform(key, vec4);
		return (*this);
	}

	RenderManager &RenderManager::setShaderUniform(Key<Shader> const &keyShader, Key<Uniform> const &key, float v)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (*this);
		shader->setUniform(key, v);
		return (*this);
	}

	RenderManager &RenderManager::setShaderUniform(Key<Shader> const &keyShader, Key<Uniform> const &key, glm::mat3 const &mat3)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (*this);
		shader->setUniform(key, mat3);
		return (*this);
	}

	Key<Sampler> RenderManager::addShaderSampler(Key<Shader> const &keyShader, std::string const &flag)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (Key<Sampler>(KEY_DESTROY));
		return (shader->addSampler(flag));
	}

	Key<Sampler> RenderManager::getShaderSampler(Key<Shader> const &keyShader, size_t target)
	{
		Shader const *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (Key<Sampler>(KEY_DESTROY));
		return (shader->getSampler(target));
	}

	RenderManager &RenderManager::setShaderSampler(Key<Shader> const &keyShader, Key<Sampler> const &keySampler, Key<Texture> const &keyTexture)
	{
		Shader *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (*this);
		Texture *texture;
		if ((texture = getTexture(keyTexture)) == NULL)
			return (*this);
		shader->setSampler(keySampler, *texture);
		return (*this);
	}

	Key<UniformBlock> RenderManager::addUniformBlock()
	{
		Key<UniformBlock> key;

		_uniformBlock[key];
		return (key);
	}

	RenderManager &RenderManager::introspectionBlock(Key<Shader> const &s, Key<InterfaceBlock> const &i, Key<UniformBlock> const &u)
	{
		Shader *shader = getShader(s);
		UniformBlock *uniformBlock = getUniformBlock(u);
		shader->introspection(i, *uniformBlock);
		return (*this);
	}

	RenderManager &RenderManager::rmUniformBlock(Key<UniformBlock> &key)
	{
		if (getUniformBlock(key) == NULL)
			return (*this);
		_uniformBlock.erase(key);
		key.destroy();
		return (*this);
	}

	Key<UniformBlock> RenderManager::getUniformBlock(size_t target) const
	{
		if (target >= _uniformBlock.size())
			assert(0);
		auto &element = _uniformBlock.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	Key<InterfaceBlock> RenderManager::addShaderInterfaceBlock(Key<Shader> const &keyShader, std::string const &flag, Key<UniformBlock> &keyUniformBlock)
	{
		Shader *shader = getShader(keyShader);
		UniformBlock *uniformBlock = getUniformBlock(keyUniformBlock);
		return (shader->addInterfaceBlock(flag, *uniformBlock));
	}

	RenderManager &RenderManager::setShaderInterfaceBlock(Key<Shader> const &keyShader, Key<InterfaceBlock> const &i, Key<UniformBlock> const &u)
	{
		Shader *shader = getShader(keyShader);
		UniformBlock *uniformBlock = getUniformBlock(u);
		shader->setInterfaceBlock(i, *uniformBlock);
		return (*this);
	}

	Key<InterfaceBlock> RenderManager::getShaderInterfaceBlock(Key<Shader> const &keyShader, size_t target)
	{
		Shader const *shader;
		if ((shader = getShader(keyShader)) == NULL)
			return (Key<InterfaceBlock>(KEY_DESTROY));
		return (shader->getInterfaceBlock(target));
	}

	Key<Texture> RenderManager::addTexture2D(GLsizei width, GLsizei height, GLenum internalFormat, bool mipmapping)
	{
		Key<Texture> key;

		_textures[key] = new Texture2D(width, height, internalFormat, mipmapping);
		return (key);
	}

	RenderManager &RenderManager::parameterTexture(Key<Texture> const &key, GLenum pname, GLint param)
	{
		Texture *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->parameter(pname, param);
		return (*this);
	}

	RenderManager &RenderManager::rmTexture(Key<Texture> &key)
	{
		Texture *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		delete texture;
		_textures.erase(key);
		key.destroy();
		return (*this);
	}

	Key<Texture> RenderManager::getTexture(size_t target) const
	{
		if (target >= _textures.size())
			assert(0);
		auto &element = _textures.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	GLenum RenderManager::getTypeTexture(Key<Texture> const &key)
	{
		Texture const *texture;
		if ((texture = getTexture(key)) == NULL)
			return (GL_NONE);
		return (texture->getType());
	}

	Shader *RenderManager::getShader(Key<Shader> const &key)
	{
		if (!key)
			assert(0);
		if (_shaders.size() == 0)
			assert(0);
		if (key == _optimizeShaderSearch.first)
			return (_optimizeShaderSearch.second);
		auto &shader = _shaders.find(key);
		if (shader == _shaders.end())
			assert(0);
		_optimizeShaderSearch.first = key;
		_optimizeShaderSearch.second = shader->second;
		return (shader->second);
	}

	Texture *RenderManager::getTexture(Key<Texture> const &key)
	{
		if (!key)
			assert(0);
		if (_textures.size() == 0)
			assert(0);
		if (key == _optimizeTextureSearch.first)
			return (_optimizeTextureSearch.second);
		auto &texture = _textures.find(key);
		if (texture == _textures.end())
			assert(0);
		_optimizeTextureSearch.first = key;
		_optimizeTextureSearch.second = texture->second;
		return (texture->second);
	}

	UniformBlock *RenderManager::getUniformBlock(Key<UniformBlock> const &key)
	{
		if (!key)
			assert(0);
		if (_uniformBlock.size() == 0)
			assert(0);
		if (key == _optimizeUniformBlockSearch.first)
			return (_optimizeUniformBlockSearch.second);
		auto &uniformBlock = _uniformBlock.find(key);
		if (uniformBlock == _uniformBlock.end())
			assert(0);
		_optimizeUniformBlockSearch.first = key;
		_optimizeUniformBlockSearch.second = &uniformBlock->second;
		return (&uniformBlock->second);
	}

	RenderPass *RenderManager::getRenderPass(Key<RenderPass> const &key)
	{
		if (!key)
			assert(0);
		if (_renderPass.size() == 0)
			assert(0);
		if (key == _optimizeRenderPassSearch.first)
			return (_optimizeRenderPassSearch.second);
		auto &renderPassIndex = _renderPass.find(key);
		if (renderPassIndex == _renderPass.end())
			assert(0);
		_optimizeRenderPassSearch.first = key;
		_optimizeRenderPassSearch.second = renderPassIndex->second;
		return (renderPassIndex->second);
	}


	RenderManager &RenderManager::uploadTexture(Key<Texture> const &key, GLenum format, GLenum type, GLvoid *img)
	{
		Texture const *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->upload(format, type, img);
		texture->generateMipMap();
		return (*this);
	}

	RenderManager &RenderManager::downloadTexture(Key<Texture> const &key, GLenum format, GLenum type, GLvoid *img)
	{
		Texture const *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->download(format, type, img);
		return (*this);
	}

	RenderManager &RenderManager::setlevelTargetTexture(Key<Texture> const &key, uint8_t levelTarget)
	{
		Texture *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->setLevelTarget(levelTarget);
		return (*this);
	}

	RenderManager &RenderManager::bindTexture(Key<Texture> const &key)
	{
		Texture const *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->bind();
		return (*this);
	}

	RenderManager &RenderManager::unbindTexture(Key<Texture> const &key) 
	{
		Texture const *texture;
		if ((texture = getTexture(key)) == NULL)
			return (*this);
		texture->unbind();
		return (*this);
	}

	Key<RenderPass> RenderManager::addRenderPass(Key<Shader> const &keyShader, glm::ivec4 const &rect)
	{
		Key<RenderPass> key;
		Shader *shader;

		if ((shader = getShader(keyShader)) == NULL)
			return (Key<RenderPass>(KEY_DESTROY));
		auto &element = _renderPass[key] = new RenderPass(*shader, geometryManager, materialManager);
		element->configRect(rect);
		return (key);
	}

	Key<RenderPass> RenderManager::getRenderPass(size_t target) const
	{
		if (target >= _renderPass.size())
			assert(0);
		auto &element = _renderPass.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	GEN_DEF_RENDER_PUSH_TASK(RenderPass);

	RenderManager &RenderManager::configRenderPass(Key<RenderPass> const &key, glm::ivec4 const &rect, GLenum mode, GLint sample)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->setMode(mode);
		renderPass->configRect(rect);
		return (*this);
	}

	RenderManager &RenderManager::pushOutputColorRenderPass(Key<RenderPass> const &key, GLenum attachement, GLenum internalFormat)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->pushColorOutput(attachement, internalFormat);
		return (*this);
	}

	RenderManager &RenderManager::popOutputColorRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->popColorOutput();
		return (*this);
	}

	RenderManager &RenderManager::pushInputColorRenderPass(Key<RenderPass> const &key, Key<Sampler> const &s)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->pushInputSampler(s);
		return (*this);
	}
	
	RenderManager &RenderManager::popInputColorRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->popInputSampler();
		return (*this);
	}

	RenderManager &RenderManager::createDepthBufferRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->createDepthBuffer();
		return (*this);
	}

	RenderManager &RenderManager::createStencilBufferRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->createStencilBuffer();
		return (*this);
	}

	RenderManager &RenderManager::useInputDepthRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->useInputDepth();
		return (*this);
	}

	RenderManager &RenderManager::unUseInputDepthRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->unUseInputDepth();
		return (*this);
	}

	RenderManager &RenderManager::useInputStencilRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->useInputStencil();
		return (*this);
	}

	RenderManager &RenderManager::unUseInputStencilRenderPass(Key<RenderPass> const &key)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->unUseInputStencil();
		return (*this);
	}

	RenderManager &RenderManager::useInputColorRenderPass(Key<RenderPass> const &key, GLenum attachement)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->useInputColor(attachement);
		return (*this);
	}

	RenderManager &RenderManager::unUseInputColorRenderPass(Key<RenderPass> const &key, GLenum attachement)
	{
		RenderPass *renderPass;

		if ((renderPass = getRenderPass(key)) == NULL)
			return (*this);
		renderPass->unUseInputColor(attachement);
		return (*this);
	}

	RenderManager &RenderManager::unbindMaterialToShader(Key<Shader> const &shaderKey, Key<Uniform> const &uniformKey)
	{
		Shader *shader;
		if ((shader = getShader(shaderKey)) == NULL)
			return (*this);
		shader->unbindMaterial(uniformKey);
		return (*this);
	}

	RenderManager &RenderManager::bindTransformationToShader(Key<Shader> const &shaderKey, Key<Uniform> const &uniformKey)
	{
		Shader *shader;
		if ((shader = getShader(shaderKey)) == NULL)
			return (*this);
		shader->bindingTransformation(uniformKey);
		return (*this);
	}

	RenderPostEffect *RenderManager::getRenderPostEffect(Key<RenderPostEffect> const &key)
	{
		if (!key)
			assert(0);
		if (_renderPostEffect.size() == 0)
			assert(0);
		if (key == _optimizeRenderPostEffectSearch.first)
			return (_optimizeRenderPostEffectSearch.second);
		auto &renderPostEffect = _renderPostEffect.find(key);
		if (renderPostEffect == _renderPostEffect.end())
			assert(0);
		_optimizeRenderPostEffectSearch.first = key;
		_optimizeRenderPostEffectSearch.second = renderPostEffect->second;
		return (renderPostEffect->second);
	}

	RenderOnScreen *RenderManager::getRenderOnScreen(Key<RenderOnScreen> const &key)
	{
		if (!key)
			assert(0);
		if (_renderOnScreen.size() == 0)
			assert(0);
		if (key == _optimizeRenderOnScreenSearch.first)
			return (_optimizeRenderOnScreenSearch.second);
		auto &renderOnScreen = _renderOnScreen.find(key);
		if (renderOnScreen == _renderOnScreen.end())
			assert(0);
		_optimizeRenderOnScreenSearch.first = key;
		_optimizeRenderOnScreenSearch.second = renderOnScreen->second;
		return (renderOnScreen->second);
	}

	Pipeline *RenderManager::getPipeline(Key<Pipeline> const &key)
	{
		if (!key)
			assert(0);
		if (_pipelines.size() == 0)
			assert(0);
		if (key == _optimizePipelineSearch.first)
			return (_optimizePipelineSearch.second);
		auto &pipeline = _pipelines.find(key);
		if (pipeline == _pipelines.end())
			assert(0);
		_optimizePipelineSearch.first = key;
		_optimizePipelineSearch.second = &pipeline->second;
		return (&pipeline->second);
	}

	Key<RenderPostEffect> RenderManager::addRenderPostEffect(Key<Shader> const &s, glm::ivec4 const &rect)
	{
		Key<RenderPostEffect> key;
		Shader *shader;

		if ((shader = getShader(s)) == NULL)
			return  (Key<RenderPostEffect>(KEY_DESTROY));
		auto &element = _renderPostEffect[key] = new RenderPostEffect(geometryManager.getSimpleForm(QUAD), *shader, geometryManager);
		element->pushInputSampler(_preShaderQuad->getSampler(0));
		element->configRect(rect);
		return (key);
	}

	Key<RenderPostEffect> RenderManager::getRenderPostEffect(size_t target) const
	{
		if (target >= _renderPass.size())
			assert(0);
		auto &element = _renderPostEffect.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	GEN_DEF_RENDER_PUSH_TASK(RenderPostEffect);

	RenderManager &RenderManager::configRenderPostEffect(Key<RenderPostEffect> const &key, glm::ivec4 const &rect, GLenum mode, GLint sample)
	{
		RenderPostEffect *renderPostEffect;

		if ((renderPostEffect = getRenderPostEffect(key)) == NULL)
			return (*this);
		renderPostEffect->setMode(mode);
		renderPostEffect->configRect(rect);
		return (*this);
	}

	RenderManager &RenderManager::pushOutputColorRenderPostEffect(Key<RenderPostEffect> const &key, GLenum attachement, GLenum internalFormat)
	{
		RenderPostEffect *renderPostEffect = getRenderPostEffect(key);
		renderPostEffect->pushColorOutput(attachement, internalFormat);
		return (*this);
	}

	RenderManager & RenderManager::popOutputColorRenderPostEffect(Key<RenderPostEffect> const &key)
	{
		RenderPostEffect *renderPostEffect = getRenderPostEffect(key);
		renderPostEffect->popColorOutput();
		return (*this);
	}

	RenderManager &RenderManager::pushInputColorRenderPostEffect(Key<RenderPostEffect> const &key, Key<Sampler> const &s)
	{
		RenderPostEffect *renderPostEffect;

		if ((renderPostEffect = getRenderPostEffect(key)) == NULL)
			return (*this);
		renderPostEffect->pushInputSampler(s);
		return (*this);
	}
	
	RenderManager &RenderManager::popInputColorRenderPostEffect(Key<RenderPostEffect> const &key)
	{
		RenderPostEffect *renderPostEffect;

		if ((renderPostEffect = getRenderPostEffect(key)) == NULL)
			return (*this);
		renderPostEffect->popInputSampler();
		return (*this);
	}

	Key<RenderOnScreen> RenderManager::addRenderOnScreen(glm::ivec4 const &rect)
	{
		Key<RenderOnScreen> key;

		createPreShaderQuad();
		auto &element = _renderOnScreen[key] = new RenderOnScreen(geometryManager.getSimpleForm(QUAD), *_preShaderQuad, geometryManager);
		element->pushInputSampler(_preShaderQuad->getSampler(0));
		element->configRect(rect);
		return (key);
	}

	Key<RenderOnScreen> RenderManager::getRenderOnScreen(size_t target) const
	{
		if (target >= _textures.size())
			assert(0);
		auto &element = _renderOnScreen.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	GEN_DEF_RENDER_PUSH_TASK(RenderOnScreen);

	RenderManager &RenderManager::branch(Key<RenderPass> const &from, Key<RenderPass> const &to)
	{
		RenderPass *renderPassFrom;
		RenderPass *renderPassTo;

		if ((renderPassFrom = getRenderPass(from)) == NULL)
			return (*this);
		if ((renderPassTo = getRenderPass(to)) == NULL)
			return (*this);
		renderPassTo->branchInput(*renderPassFrom);
		return (*this);
	}

	RenderManager &RenderManager::branch(Key<RenderPass> const &from, Key<RenderPostEffect> const &to)
	{
		RenderPass *renderPassFrom;
		RenderPostEffect *renderPassTo;

		if ((renderPassFrom = getRenderPass(from)) == NULL)
			return (*this);
		if ((renderPassTo = getRenderPostEffect(to)) == NULL)
			return (*this);
		renderPassTo->branchInput(*renderPassFrom);
		return (*this);
	}

	RenderManager &RenderManager::branch(Key<RenderPass> const &from, Key<RenderOnScreen> const &to)
	{
		RenderPass *renderPassFrom;
		RenderOnScreen *renderPassTo;

		if ((renderPassFrom = getRenderPass(from)) == NULL)
			return (*this);
		if ((renderPassTo = getRenderOnScreen(to)) == NULL)
			return (*this);
		renderPassTo->branchInput(*renderPassFrom);
		return (*this);
	}

	RenderManager &RenderManager::branch(Key<RenderPostEffect> const &from, Key<RenderOnScreen> const &to)
	{
		RenderPostEffect *renderPassFrom;
		RenderOnScreen *renderPassTo;

		if ((renderPassFrom = getRenderPostEffect(from)) == NULL)
			return (*this);
		if ((renderPassTo = getRenderOnScreen(to)) == NULL)
			return (*this);
		renderPassTo->branchInput(*renderPassFrom);
		return (*this);
	}

	Key<Pipeline> RenderManager::addPipeline()
	{
		Key<Pipeline> key;

		_pipelines[key];
		return (key);
	}

	RenderManager &RenderManager::setPipeline(Key<Pipeline> const &p, uint8_t time, Key<RenderPass> const &r)
	{
		Pipeline *pipeline;
		RenderPass *renderPass;

		if ((pipeline = getPipeline(p)) == NULL)
			return (*this);
		if ((renderPass = getRenderPass(r)) == NULL)
			return (*this);
		pipeline->setRendering(time, renderPass);
		return (*this);
	}

	RenderManager &RenderManager::setPipeline(Key<Pipeline> const &p, uint8_t time, Key<RenderPostEffect> const &r)
	{
		Pipeline *pipeline;
		RenderPostEffect *renderPostEffect;

		if ((pipeline = getPipeline(p)) == NULL)
			return (*this);
		if ((renderPostEffect = getRenderPostEffect(r)) == NULL)
			return (*this);
		pipeline->setRendering(time, renderPostEffect);
		return (*this);
	}

	RenderManager &RenderManager::setPipeline(Key<Pipeline> const &p, uint8_t time, Key<RenderOnScreen> const &r)
	{
		Pipeline *pipeline;
		RenderOnScreen *renderOnScreen;

		if ((pipeline = getPipeline(p)) == NULL)
			return (*this);
		if ((renderOnScreen = getRenderOnScreen(r)) == NULL)
			return (*this);
		pipeline->setRendering(time, renderOnScreen);
		_minTime = std::min(_minTime, pipeline->getMinTime());
		_maxTime = std::max(_maxTime, pipeline->getMaxTime());
		return (*this);
	}

	Key<Pipeline> RenderManager::getPipeline(size_t target)
	{
		if (target >= _pipelines.size())
			assert(0);
		auto &element = _pipelines.begin();
		for (size_t index = 0; index < target; ++index)
			++element;
		return (element->first);
	}

	RenderManager &RenderManager::updatePipeline(Key<Pipeline> const &p, AGE::Vector<AGE::Drawable> const &objectRender)
	{
		Pipeline *pipeline;

		if ((pipeline = getPipeline(p)) == NULL)
			return (*this);
		pipeline->setToRender(objectRender);
		return (*this);
	}

	RenderManager &RenderManager::drawPipelines()
	{
		for (uint8_t time = _minTime; time < _maxTime; ++time)
			for (auto &pipeline = _pipelines.begin(); pipeline != _pipelines.end(); ++pipeline)
				pipeline->second.draw(time);
		return (*this);
	}

	RenderManager &RenderManager::drawPipeline(Key<Pipeline> const &key, AGE::Vector<AGE::Drawable> const &objectRender)
	{
		Pipeline *pipeline;

		if ((pipeline = getPipeline(key)) == NULL)
			return (*this);
		pipeline->setToRender(objectRender);
		for (uint8_t time = pipeline->getMinTime(); time < pipeline->getMaxTime(); ++time)
			pipeline->draw(time);
		return (*this);
	}

	RenderManager &RenderManager::draw(Key<RenderOnScreen> const &o, Key<RenderPass> const &r, AGE::Vector<AGE::Drawable> const &objectRender)
	{
		RenderOnScreen *renderOnScreen;
		RenderPass *renderPass;

		if ((renderOnScreen = getRenderOnScreen(o)) == NULL)
			return (*this);
		if ((renderPass = getRenderPass(r)) == NULL)
			return (*this);
		renderPass->setRenderPassObjects(objectRender);
		renderOnScreen->branchInput(*renderPass);
		renderPass->draw();
		renderOnScreen->draw();
		return (*this);
	}
}