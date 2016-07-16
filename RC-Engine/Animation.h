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
		struct BoneInfo
		{
			aiMatrix4x4 offset;
			aiMatrix4x4 finalTransformation;

			BoneInfo()
			{
				offset = aiMatrix4x4();
				finalTransformation = aiMatrix4x4();
			}
		};

		Assimp::Importer importer;
		const aiScene * scene;
		std::vector<BoneInfo> bones;
		uint32_t numBones;
		aiMatrix4x4 globalInverseTransform;
		std::vector<aiMatrix4x4> boneTransforms;
		std::vector<glm::mat4> boneTransformsGLM;
		std::map<std::string, uint32_t> boneMapping;

		float runTime;
		float speed;

		bool loaded;
	private:
		void ReadNodeHierarchy(float animTime, const aiNode* node, const aiMatrix4x4& parentTransform);
		const aiNodeAnim * FindNodeAnim(std::string nodeName);
		aiMatrix4x4 InterpolateTranslation(float time, const aiNodeAnim* nodeAnim);
		aiMatrix4x4 InterpolateRotation(float time, const aiNodeAnim* nodeAnim);
		aiMatrix4x4 InterpolateScale(float time, const aiNodeAnim* nodeAnim);
	public:
		Animation();
		~Animation();

		bool Init(std::string filename);
		void SetAnimationSpeed(float speed);
		bool IsLoaded();
		void Update(float time);
		std::vector<glm::mat4>& GetBoneTransforms();
};