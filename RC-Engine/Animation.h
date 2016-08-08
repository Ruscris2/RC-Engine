/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Animation.h                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define MAX_BONES 64

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <map>
#include <gtc/type_ptr.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Animation
{
	private:
		Assimp::Importer importer;
		const aiScene * scene;
		uint32_t numBones;
		aiMatrix4x4 globalInverseTransform;
		std::vector<aiMatrix4x4> boneTransforms;
		std::vector<glm::mat4> boneTransformsGLM;

		float runTime;
		float speed;
		bool loop;
		bool isFinished;
	private:
		void ReadNodeHierarchy(float animTime, const aiNode* node, const aiMatrix4x4& parentTransform,
			std::vector<aiMatrix4x4>& boneOffsets, std::map<std::string, uint32_t>& boneMapping);
		const aiNodeAnim * FindNodeAnim(std::string nodeName);
		aiMatrix4x4 InterpolateTranslation(float time, const aiNodeAnim* nodeAnim);
		aiMatrix4x4 InterpolateRotation(float time, const aiNodeAnim* nodeAnim);
		aiMatrix4x4 InterpolateScale(float time, const aiNodeAnim* nodeAnim);
	public:
		Animation();
		~Animation();

		bool Init(std::string filename, uint32_t numBones, bool loopAnim);
		void SetAnimationSpeed(float speed);
		void ResetAnimation();
		void Update(float time, std::vector<aiMatrix4x4>& boneOffsets, std::map<std::string, uint32_t>& boneMapping);
		std::vector<glm::mat4>& GetBoneTransforms();
		bool IsFinished();
};