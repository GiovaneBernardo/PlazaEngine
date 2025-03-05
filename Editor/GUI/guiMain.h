#pragma once
#include "ThirdParty/GLFW/include/GLFW/glfw3.h"
#include "Engine/Components/Core/Entity.h"
#include "Editor/GUI/GuiWindow.h"
#include "Editor/EditorTools/EditorTool.h"

namespace ImGuizmoHelper {
	static bool IsDrawing;
}

using namespace std;
namespace Plaza {
	class Camera;
	struct TrackedImage;
	namespace Editor {
		class HierarchyWindow;
		class AssetsImporterWindow;
		class Console;
		class RenderGraphEditor;
		class Gui {
		  public:
			static inline EditorStyle* sEditorStyle;

			class FileExplorer;
			class MainMenuBar;
			class Hierarchy;
			// class MaterialInspector;
			class TransformInspector;

			static inline ImGuiContext* mMainContext = nullptr;
			static inline ImGuiContext* mMainProgressBarContext = nullptr;

			static void setupDockspace(Scene* scene, GLFWwindow* window, Camera* camera);
			static void changeSelectedGameObject(Entity* newSelectedGameObject);
			static void Init(GLFWwindow* window);
			static void Delete();
			static void Update();
			static void NewFrame();

			static void beginScene(Scene* scene, Camera& camera);
			static void beginEditor(Scene* scene, Camera& camera);
			static void beginHierarchyView(Scene* scene);
			static void beginInspector(Scene* scene, Camera camera);
			static void beginImageInspector(Scene* scene, Camera camera);
			static void beginAssetsViewer(Scene* scene, Camera camera);
			static void beginProfiler(Scene* scene);
			static void beginMainProgressBar(float percentage);

			static inline bool mImageInspectorShowAllImages = false;
			static inline bool mShowSelectedImageInEditorView = false;
			static inline bool mFlipY = true;
			static inline TrackedImage* mSelectedImageInspector = nullptr;
			static inline ImVec2 imageSize = ImVec2(200, 200);

			static string scenePayloadName;

			static bool sceneViewUsingEditorCamera;
			static bool isHierarchyOpen;
			static bool isSceneOpen;
			static bool isInspectorOpen;
			static bool isFileExplorerOpen;
			static bool canUpdateContent;

			static inline bool sRenderingTextureViewer = false;

			static ImTextureID playPauseButtonImageId;

			static void UpdateSizes();

			static void OpenAssetImporterContext(std::string fileToImport);

			static inline GuiLayer sFocusedLayer = GuiLayer::SCENE;
			static inline std::vector<GuiWindow> mGuiWindows = std::vector<GuiWindow>();

			HierarchyWindow* mHierarchy;
			AssetsImporterWindow* mAssetsImporter;
			Console* mConsole;
			RenderGraphEditor* mRenderGraphEditor;

			static inline std::map<EditorTool::ToolType, std::unique_ptr<EditorTool>> sEditorTools;
			// static inline EditorTool::ToolType sEditorToolCaptureMouseClick;
			// static inline EditorTool::ToolType sEditorToolCaptureKeyPress;
		  private:
			static void CommonGuiInit(GLFWwindow* window, EditorStyle* editorStyle, bool initializePlazaImGuiPools);
			static inline bool isAssetImporterOpen = false;
			static inline std::string mAssetToImportPath = "";
		};
	} // namespace Editor
} // namespace Plaza

/// <summary>
/// ImGui Helper
/// </summary>
namespace ImGui {
	inline bool Compare(ImVec2 firstVec, ImVec2 secondVec) {
		return firstVec.x == secondVec.x && firstVec.y == secondVec.y;
	}
	// Transforms glm::vec2 to ImVec2
	inline ImVec2 imVec2(glm::vec2 vec) { return ImVec2(vec.x, vec.y); }
	// Transforms ImVec2 to glm::vec2
	inline glm::vec2 glmVec2(ImVec2 imguiVec) { return glm::vec2(imguiVec.x, imguiVec.y); }

} // namespace ImGui
