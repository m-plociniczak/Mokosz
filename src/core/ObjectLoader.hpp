#pragma once

#include <tiny_obj_loader.h>
#include <memory>
#include <iostream>
#include <exception>
#include <renderer/Mesh.h>

namespace ObjLoader
{
    std::shared_ptr<Mesh> loadObjMesh(const std::string& path);
}