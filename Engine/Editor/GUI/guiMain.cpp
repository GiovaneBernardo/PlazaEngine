#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

#include <stb/stb_image.h>

#include <iostream>
#include <random>
#include <filesystem>
#include <fileSystem/fileSystem.h>
#include "guiMain.h"
#include "../../Application.h"

void beginScene(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes);
void beginHierarchyView(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes);
void beginInspector(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes);

void gui::setupDockspace(GLFWwindow* window, int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes) {
	ImGuiWindowFlags  windowFlags = ImGuiWindowFlags_MenuBar;
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.appSize));
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));
	if (ImGui::Begin("mainRect", new bool(true), windowFlags)) {
		ImGui::PopStyleVar();
		// Create UI elements
		beginScene(gameFrameBuffer, appSizes, lastAppSizes);
		beginHierarchyView(gameFrameBuffer, appSizes, lastAppSizes);
		beginInspector(gameFrameBuffer, appSizes, lastAppSizes);
	}
	appSizes.sceneSize = glm::vec2(appSizes.appSize.x - appSizes.hierarchySize.x - appSizes.inspectorSize.x, appSizes.sceneSize.y);
	ImGui::End();
}

// Create the scene view
inline void beginScene(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes) {
	ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.sceneSize));
	ImGui::SetNextWindowPos(ImVec2(appSizes.hierarchySize.x, 0));
	ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
	if (ImGui::Begin("Scene", new bool(true), sceneWindowFlags)) {
		// set this window image to be the game framebuffer
		ImGui::SetWindowPos(ImVec2(appSizes.hierarchySize.x, 0));
		int textureId = gameFrameBuffer; 
		ImGui::Image(ImTextureID(textureId), ImGui::imVec2(appSizes.sceneSize));
	}
	
	ImGui::End();
	ImGui::PopStyleVar();
}

void beginHierarchyView(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes) {
	ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.hierarchySize), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red//

	if (ImGui::Begin("Hierarchy", new bool(true), sceneWindowFlags)) {
		ImGui::SetWindowSize(ImVec2(appSizes.hierarchySize.x, appSizes.hierarchySize.y), ImGuiCond_Always);
		// set this window image to be the game framebuffer
		//std::cout << appSizes.hierarchyWidth << std::endl;
		const ImVec2& CurSize = ImGui::GetWindowSize();
		if (CurSize.x != lastAppSizes.hierarchySize.x || CurSize.y != lastAppSizes.hierarchySize.y) {
			appSizes.hierarchySize = ImGui::glmVec2(CurSize);
			lastAppSizes.hierarchySize = appSizes.hierarchySize;
			ImGui::SetWindowSize(ImVec2(appSizes.hierarchySize.x, appSizes.hierarchySize.y), ImGuiCond_Always);
			ImGui::SetWindowPos(ImVec2(0, 0));
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void beginInspector(int gameFrameBuffer, AppSizes& appSizes, AppSizes& lastAppSizes) {
	ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.inspectorSize), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(appSizes.appSize.x - appSizes.inspectorSize.x, 0));

	ImGuiWindowFlags  inspectorWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red//
	if (ImGui::Begin("Inspector", new bool(true), inspectorWindowFlags)) {

		const ImVec2& CurSize = ImGui::GetWindowSize();
		std::cout << CurSize.x << std::endl;
		std::cout << lastAppSizes.inspectorSize.x << std::endl;
		if (!ImGui::Compare(CurSize, ImGui::imVec2(lastAppSizes.inspectorSize))) {
			appSizes.inspectorSize = ImGui::glmVec2(CurSize);
			lastAppSizes.inspectorSize = appSizes.inspectorSize;
			ImGui::SetWindowSize(ImGui::imVec2(appSizes.inspectorSize), ImGuiCond_Always);
			ImGui::SetWindowPos(ImVec2(appSizes.appSize.x - appSizes.inspectorSize.x, 0));
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}



//glm::vec2 ImGui::vec2() {
//	return glm::vec2;
//}