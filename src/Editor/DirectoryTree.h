#pragma once
#include <filesystem>
#include <string>

struct DirectoryNode
{
	std::string FullPath;
	std::string FileName;
	std::vector<DirectoryNode> Children;
	bool IsDirectory;
    bool Selected;
};

static DirectoryNode project_root_node;

static DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath);
static void ImplementDirectoryNode();
static void SearchOnCodeEditor();