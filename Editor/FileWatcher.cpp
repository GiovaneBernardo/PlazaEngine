#include "Engine/Core/PreCompiledHeaders.h"
#include "Filewatcher.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/FileExplorer/RenamedFile.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
namespace Plaza::Editor {
	std::unordered_map<uint64_t, std::function<void()>> Filewatcher::mMainThreadQueue = std::unordered_map<uint64_t, std::function<void()>>();
	std::map< filewatch::Event, std::string> Filewatcher::mQueuedEvents = std::map<filewatch::Event, std::string>();
	std::queue<std::function<void()>> Filewatcher::taskQueue = std::queue<std::function<void()>>();
	std::mutex Filewatcher::queueMutex = std::mutex();

	struct FileWatchEvent {
		std::string path;
		filewatch::Event event;
	};

	static FileWatchEvent sLastFileWatchEvent{};

	void Filewatcher::Start(std::string pathToWatch) {
		filewatch::FileWatch<std::string>* watch = new filewatch::FileWatch<std::string>(pathToWatch, [pathToWatch](const std::string path, const filewatch::Event changeType) {
			//std::cout << path << " - ";
			std::filesystem::path fsPath = std::filesystem::path{ path };
			std::string finalPath = pathToWatch + "\\" + path;
			mQueuedEvents.emplace(changeType, finalPath);
			switch (changeType) {
			case filewatch::Event::added:
				if (fsPath.extension() == ".cs") {
					Application::Get()->activeProject->scripts.emplace(pathToWatch + "\\" + path, Script());
				}

				//if (fsPath.extension().string().starts_with(Standards::engineExtName))
				//	AssetsLoader::LoadAsset(AssetsManager::LoadFileAsAsset(finalPath));
				//else
				//	AssetsImporter::ImportAsset(finalPath);

				break;
			case filewatch::Event::removed:
				if (fsPath.extension() == ".cs") {
					Application::Get()->activeProject->scripts.erase(pathToWatch + "\\" + path);
				}
				break;
			case filewatch::Event::modified:
				if (fsPath.extension() == ".cs") {
					// TODO: For now its just recompiling the scripting but not hot reloading
					if (Filewatcher::mMainThreadQueue.size() <= 0) {
						Filewatcher::AddToMainThread([finalPath]() {
							ScriptManager::RecompileDll("", "");
							std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> allFields = FieldManager::GetAllScritpsFields();
							Mono::OnStartAll(false);
							FieldManager::ApplyAllScritpsFields(allFields);
							});
						//Filewatcher::AddToMainThread([finalPath]() {ScriptManager::ReloadSpecificAssembly(finalPath); });
					}
					//Filewatcher::AddToMainThread([finalPath]() {ScriptManager::RecompileDll("", ""); });
				}
				break;
			case filewatch::Event::renamed_old: // The file was renamed and this is the old name
				sLastFileWatchEvent = { finalPath, changeType };

				break;
			case filewatch::Event::renamed_new: // The file was renamed and this is the new name
				if (AssetsManager::HasAssetPath(sLastFileWatchEvent.path)) {
					auto it = AssetsManager::mAssetsUuidByPath.find(sLastFileWatchEvent.path);
					AssetsManager::mAssetsUuidByPath.emplace(finalPath, it->second);
					AssetsManager::mAssetsUuidByPath.erase(it);
				}
				break;
			};

			//Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);

			Filewatcher::AddToMainThread([]() {
				Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
				});
			});
	}

	void Filewatcher::UpdateOnMainThread() {

		std::function<void()> task;

		{
			std::lock_guard<std::mutex> lock(queueMutex);

			if (!taskQueue.empty()) {
				task = taskQueue.front();
				taskQueue.pop();
			}
		}

		if (task) {
			task(); // Execute the task on the main thread
		}
		/* Execute queued functions*/
		//for (auto [key, function] : Filewatcher::mMainThreadQueue)
		//{
		//	function();
		//	//Filewatcher::mMainThreadQueue.erase(Filewatcher::mMainThreadQueue.find(key));
		//	//Filewatcher::mMainThreadQueue.erase(key);
		//	//mMainThreadQueue.erase(mMainThreadQueue.find(function));
		//}
		//Filewatcher::mMainThreadQueue.clear();
		/* Check the queued events, to check for renamed files */
		if (mQueuedEvents.size() >= 3) {
			std::string newPath;
			std::string oldPath;
			bool renamed = false;
			for (auto [event, path] : mQueuedEvents) {
				if (event == filewatch::Event::renamed_old || event == filewatch::Event::renamed_new || event == filewatch::Event::modified)
					renamed = true;
				else
					renamed = false;

				if (event == filewatch::Event::renamed_old)
					oldPath = path;
				if (event == filewatch::Event::renamed_new)
					newPath = path;
			}

			if (renamed) {
				Editor::RenamedFileManager::Run(oldPath, newPath);
			}
		}
		mQueuedEvents.clear();
	}

	void Filewatcher::AddToMainThread(const std::function<void()>& function) {
		//mMainThreadQueue.emplace(Plaza::UUID::NewUUID(), function);
		std::lock_guard<std::mutex> lock(queueMutex);
		taskQueue.push(function);
	}
}