import uuid
import json
import xml.etree.ElementTree as ET
import os
import copy

sceneXml = "D:/CppProjects/JoyEngine/MitsubaXmlParser/data/scene.xml"
materialsPath = "D:/CppProjects/JoyEngine/JoyData/materials/"
scenePath = "D:/CppProjects/JoyEngine/JoyData/scenes/"
dataDBPath = "D:/CppProjects/JoyEngine/JoyData/"

materials = {}
textures = {}
meshes = {}
objects = []

tree = ET.parse(sceneXml)
root = tree.getroot()
for asset in root:
    if asset.tag == "sensor" or asset.tag == "integrator":
        continue
    if asset.tag == "bsdf":
        materialName = ""
        color = []
        texture = ""
        if asset.attrib['type'] == "twosided" or asset.attrib['type'] == "bumpmap":
            material = None

            if asset.attrib['type'] == "twosided":
                materialName = asset.attrib['id']
                material = asset[0]
            else:
                if asset.attrib['type'] == "bumpmap":
                    materialName = asset[1].attrib['id']
                    material = asset[1][0]

            if material.attrib['type'] == "diffuse" \
                    or material.attrib['type'] == "roughplastic" \
                    or material.attrib['type'] == "roughconductor" \
                    or material.attrib['type'] == "plastic" \
                    or material.attrib['type'] == "conductor":
                for materialAttribute in material:
                    if materialAttribute.tag == "texture":
                        for arg in materialAttribute:
                            if arg.attrib["name"] == "filename":
                                texture = arg.attrib["value"]
                                if texture not in textures.keys():
                                    textures[texture] = str(uuid.uuid4())
                    if materialAttribute.tag == "rgb" and \
                            (materialAttribute.attrib["name"] == "specularReflectance"
                             or materialAttribute.attrib["name"] == "diffuseReflectance"
                             or materialAttribute.attrib["name"] == "reflectance"):
                        for i in materialAttribute.attrib["value"].split(", "):
                            color.append(float(i))
                        if len(color) == 3:
                            color.append(1)
            else:
                raise Exception("Unhandled material type: " + material.attrib['type'])

        else:
            if asset.attrib['type'] == "roughdielectric" \
                    or asset.attrib['type'] == "dielectric" \
                    or asset.attrib['type'] == "mask" \
                    or asset.attrib['type'] == "thindielectric":
                materialName = asset.attrib['id']
            else:
                raise Exception("Unhandled bsdf type: " + asset.attrib['type'])

        if len(color) == 0:
            color = [1, 1, 1, 1]
        materials[materialName] = (color, texture, str(uuid.uuid4()))
        print(materialName, materials[materialName])

    if asset.tag == "shape":
        filename = ""
        name = ""
        position = []
        materialName = ""
        if asset.attrib['type'] == "obj":
            for objAttrib in asset:
                if objAttrib.tag == "string":
                    filename = objAttrib.attrib["value"]
                    name = filename.split("/")[1][:-4]
                    if filename not in meshes.keys():
                        meshes[filename] = str(uuid.uuid4())
                else:
                    if objAttrib.tag == "transform":
                        matrix = objAttrib[0].attrib["value"].split(" ")
                        position = [float(matrix[i * 4 + 3]) for i in range(0, 4)]
                    else:
                        if objAttrib.tag == "ref":
                            materialName = objAttrib.attrib["id"]
            if materialName not in materials.keys():
                raise Exception("There is no such material: " + materialName)
            objects.append((filename, name, position, materialName))
            print(filename, name, position, materialName)

materialJson = {
    "type": "material",
    "sharedMaterial": "74eac505-944d-45af-acdf-51bcb04fee30",
    "bindings": [
        {
            "name": "mainTexture",
            "data": "7e50aa82-5696-428c-a088-538fb78c0ee6"
        },
        {
            "name": "color",
            "data": {
                "r": 1,
                "g": 1,
                "b": 1,
                "a": 1
            }
        }
    ]
}

for materialName in materials.keys():
    f = open(os.path.join(materialsPath, materialName + ".json"), 'w+')
    materialJsonCopy = copy.deepcopy(materialJson)
    if materials[materialName][1] != "":
        materialJsonCopy["bindings"][0]["data"] = textures[materials[materialName][1]]
    else:
        materialJsonCopy["bindings"][0]["data"] = ""

    materialJsonCopy["bindings"][1]["data"]["r"] = materials[materialName][0][0]
    materialJsonCopy["bindings"][1]["data"]["g"] = materials[materialName][0][1]
    materialJsonCopy["bindings"][1]["data"]["b"] = materials[materialName][0][2]
    materialJsonCopy["bindings"][1]["data"]["a"] = materials[materialName][0][3]
    f.write(json.dumps(materialJsonCopy))
    f.close()

gameObjectJson = {
    "name": "room",
    "transform": {
        "localPosition": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        },
        "localRotation": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        },
        "localScale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
        }
    },
    "components": [
        {
            "type": "renderer",
            "model": "fc972e06-70d9-4271-8615-825003b43f74",
            "material": "70cfb3fd-9105-4987-b37a-ad1f74529f9d"
        }
    ]
}

sceneJson = {
    "type": "scene",
    "name": "roomScene",
    "objects": [
        {
            "name": "camera",
            "transform": {
                "localPosition": {
                    "x": 0.0,
                    "y": 2.0,
                    "z": -2.0
                },
                "localRotation": {
                    "x": 405.0,
                    "y": 0.0,
                    "z": 0.0
                },
                "localScale": {
                    "x": 1.0,
                    "y": 1.0,
                    "z": 1.0
                }
            },
            "components": [
                {
                    "type": "camera"
                }
            ]
        }
    ]
}

for obj in objects:
    gameObjectJsonCopy = copy.deepcopy(gameObjectJson)
    gameObjectJsonCopy["name"] = obj[1]
    gameObjectJsonCopy["transform"]["localPosition"]["x"] = obj[2][0]
    gameObjectJsonCopy["transform"]["localPosition"]["y"] = obj[2][1]
    gameObjectJsonCopy["transform"]["localPosition"]["z"] = obj[2][2]

    gameObjectJsonCopy["components"][0]["model"] = meshes[obj[0]]
    gameObjectJsonCopy["components"][0]["material"] = materials[obj[3]][2]

    sceneJson["objects"].append(gameObjectJsonCopy)

f = open(os.path.join(scenePath, "kitchen.json"), 'w+')
f.write(json.dumps(sceneJson))
f.close()

dataJson = {
    "type": "database",
    "database": [
        {"guid": "11dcfeba-c2b6-4c2e-a3c7-51054ff06f1d", "path": "scenes/kitchen.json"},
        {
            "guid": "183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34",
            "path": "shaders/shader.shader"
        },
        {
            "guid": "74eac505-944d-45af-acdf-51bcb04fee30",
            "path": "shared_materials/vikingSharedMaterial.json"
        },
    ]
}

for texture in textures.keys():
    dataJson["database"].append({
        "guid": textures[texture],
        "path": texture
    })

for materialName in materials.keys():
    dataJson["database"].append({
        "guid": materials[materialName][2],
        "path": "materials/" + materialName + ".json"
    })

for mesh in meshes.keys():
    dataJson["database"].append({
        "guid": meshes[mesh],
        "path": mesh
    })

f = open(os.path.join(dataDBPath, "data.db"), 'w+')
f.write(json.dumps(dataJson))
f.close()
