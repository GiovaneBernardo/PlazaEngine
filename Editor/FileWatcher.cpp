#include "Engine/Core/PreCompiledHeaders.h"
#include "Filewatcher.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/FileExplorer/RenamedFile.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/Scene.h"
#include <unordered_map>

namespace Plaza::Editor {
	std::unordered_map<uint64_t, std::function<void()>> Filewatcher::mMainThreadQueue =
		std::unordered_map<uint64_t, std::function<void()>>();
	std::map<filewatch::Event, std::string> Filewatcher::mQueuedEvents = std::map<filewatch::Event, std::string>();
	std::queue<std::function<void()>> Filewatcher::taskQueue = std::queue<std::function<void()>>();
	std::mutex Filewatcher::queueMutex = std::mutex();

	struct FileWatchEvent {
		std::string path;
		filewatch::Event event;
	};

	static FileWatchEvent sLastFileWatchEvent{};

	void Filewatcher::Start(std::string pathToWatch) {
		filewatch::FileWatch<std::string>* watch = new filewatch::FileWatch<std::string>(
			pathToWatch, [pathToWatch](const std::string path, const filewatch::Event changeType) {
				// std::cout << path << " - ";
				std::filesystem::path fsPath = std::filesystem::path{path};
				std::string finalPath = pathToWatch + "/" + path;
				mQueuedEvents.emplace(changeType, finalPath);

				if (finalPath.find("~") != std::string::npos)
					return;

				switch (changeType) {
					case filewatch::Event::added:
						if (fsPath.extension() == ".cs") {
							Application::Get()->activeProject->scripts.emplace(pathToWatch + "/" + path, Script());
						}

						// if (fsPath.extension().string().starts_with(Standards::engineExtName))
						//	AssetsLoader::LoadAsset(AssetsManager::LoadFileAsAsset(finalPath));
						// else
						//	AssetsImporter::ImportAsset(finalPath);

						break;
					case filewatch::Event::removed:
						if (fsPath.extension() == ".cs") {
							Application::Get()->activeProject->scripts.erase(pathToWatch + "/" + path);
						}
						break;
					case filewatch::Event::modified:
						break;
					case filewatch::Event::renamed_old: // The file was renamed and this is the old name
						sLastFileWatchEvent = {finalPath, changeType};

						break;
					case filewatch::Event::renamed_new: // The file was renamed and this is the new name
						std::filesystem::path metadataFile(sLastFileWatchEvent.path);
						metadataFile.replace_extension(Standards::metadataExtName);
						std::string metadataPath = metadataFile.string();
						if (AssetsManager::HasAssetPath(sLastFileWatchEvent.path)) {
							Asset* asset = AssetsManager::GetAsset(sLastFileWatchEvent.path);
							bool containsMetaData = std::filesystem::exists(metadataPath);
							if (containsMetaData)
								AssetsManager::RenameMetaData(asset, sLastFileWatchEvent.path, finalPath);
							else
								AssetsManager::RenameAsset(asset, sLastFileWatchEvent.path, finalPath);
						}
						else {
							if (AssetsManager::HasAssetPath(metadataPath)) {
								Asset* asset = AssetsManager::GetAsset(metadataPath);
								AssetsManager::RenameMetaData(asset, sLastFileWatchEvent.path, finalPath);
							}
						}
				};

				// Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);

				Filewatcher::AddToMainThread(
					[]() { Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory); });
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
		// for (auto [key, function] : Filewatcher::mMainThreadQueue)
		//{
		//	function();
		//	//Filewatcher::mMainThreadQueue.erase(Filewatcher::mMainThreadQueue.find(key));
		//	//Filewatcher::mMainThreadQueue.erase(key);
		//	//mMainThreadQueue.erase(mMainThreadQueue.find(function));
		// }
		// Filewatcher::mMainThreadQueue.clear();
		/* Check the queued events, to check for renamed files */
		if (mQueuedEvents.size() >= 3) {
			std::string newPath;
			std::string oldPath;
			bool renamed = false;
			for (auto [event, path] : mQueuedEvents) {
				if (event == filewatch::Event::renamed_old || event == filewatch::Event::renamed_new ||
					event == filewatch::Event::modified)
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
		// mMainThreadQueue.emplace(Plaza::UUID::NewUUID(), function);
		std::lock_guard<std::mutex> lock(queueMutex);
		taskQueue.push(function);
	}
} // namespace Plaza::Editor
