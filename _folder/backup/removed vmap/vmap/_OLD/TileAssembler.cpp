// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "WorldModel.h"
#include "TileAssembler.h"
#include "MapTree.h"
#include "BIH.h"
#include "VMapDefinitions.h"

#include <set>
#include <iomanip>
#include <sstream>
#include <iomanip>

using G3D::Vector3;
using G3D::AABox;
using G3D::inf;
using std::pair;

template<> struct BoundsTrait<VMAP::ModelSpawn*>
{
    static void getBounds(const VMAP::ModelSpawn* const &obj, G3D::AABox& out) { out = obj->getBounds(); }
};

namespace VMAP
{
    bool readChunk(FILE *rf, char *dest, const char *compare, uint32 len)
    {
        if (fread(dest, sizeof(char), len, rf) != len) return false;
        return memcmp(dest, compare, len) == 0;
    }

    Vector3 ModelPosition::transform(const Vector3& pIn) const
    {
        Vector3 out = pIn * iScale;
        out = iRotation * out;
        return(out);
    }

    //=================================================================

    TileAssembler::TileAssembler(const std::string& pSrcDirName, const std::string& pDestDirName)
    {
        iCurrentUniqueNameId = 0;
        iFilterMethod = NULL;
        iSrcDir = pSrcDirName;
        iDestDir = pDestDirName;
        //mkdir(iDestDir);
        //init();
    }

    TileAssembler::~TileAssembler()
    {
        //delete iCoordModelMapping;
    }

    bool TileAssembler::convertWorld2()
    {
        std::set<std::string> spawnedModelFiles;
        bool success = readMapSpawns();
        if (!success)
            return false;

        // export Map data
        for (MapData::iterator map_iter = mapData.begin(); map_iter != mapData.end() && success; ++map_iter)
        {
            // build global map tree
            std::vector<ModelSpawn*> mapSpawns;
            UniqueEntryMap::iterator entry;
            printf("Calculating model bounds for map %u...\n", map_iter->first);
            for (entry = map_iter->second->UniqueEntries.begin(); entry != map_iter->second->UniqueEntries.end(); ++entry)
            {
                // M2 models don't have a bound set in WDT/ADT placement data, i still think they're not used for LoS at all on retail
                if (entry->second.flags & MOD_M2)
                {
                    if (!calculateTransformedBound(entry->second))
                        break;
                }
                else if (entry->second.flags & MOD_WORLDSPAWN) // WMO maps and terrain maps use different origin, so we need to adapt :/
                {
                    // TODO: remove extractor hack and uncomment below line:
                    //entry->second.iPos += Vector3(533.33333f*32, 533.33333f*32, 0.f);
                    entry->second.iBound = entry->second.iBound + Vector3(533.33333f*32, 533.33333f*32, 0.f);
                }
                mapSpawns.push_back(&(entry->second));
                spawnedModelFiles.insert(entry->second.name);
            }

            printf("Creating map tree...\n");
            BIH pTree;
            pTree.build(mapSpawns, BoundsTrait<ModelSpawn*>::getBounds);

            // ===> possibly move this code to StaticMapTree class
            std::map<uint32, uint32> modelNodeIdx;
            for (uint32 i=0; i<mapSpawns.size(); ++i)
                modelNodeIdx.insert(pair<uint32, uint32>(mapSpawns[i]->ID, i));

            // write map tree file
            std::stringstream mapfilename;
            mapfilename << iDestDir << "/" << std::setfill('0') << std::setw(3) << map_iter->first << ".vmtree";
            FILE *mapfile = fopen(mapfilename.str().c_str(), "wb");
            if (!mapfile)
            {
                success = false;
                printf("Cannot open %s\n", mapfilename.str().c_str());
                break;
            }

            //general info
            if (success && fwrite(VMAP_MAGIC, 1, 8, mapfile) != 8) success = false;
            uint32 globalTileID = StaticMapTree::packTileID(65, 65);
            pair<TileMap::iterator, TileMap::iterator> globalRange = map_iter->second->TileEntries.equal_range(globalTileID);
            char isTiled = globalRange.first == globalRange.second; // only maps without terrain (tiles) have global WMO
            if (success && fwrite(&isTiled, sizeof(char), 1, mapfile) != 1) success = false;
            // Nodes
            if (success && fwrite("NODE", 4, 1, mapfile) != 1) success = false;
            if (success) success = pTree.writeToFile(mapfile);
            // global map spawns (WDT), if any (most instances)
            if (success && fwrite("GOBJ", 4, 1, mapfile) != 1) success = false;

            for (TileMap::iterator glob=globalRange.first; glob != globalRange.second && success; ++glob)
            {
                success = ModelSpawn::writeToFile(mapfile, map_iter->second->UniqueEntries[glob->second]);
            }

            fclose(mapfile);

            // <====

            // write map tile files, similar to ADT files, only with extra BSP tree node info
            TileMap &tileEntries = map_iter->second->TileEntries;
            TileMap::iterator tile;
            for (tile = tileEntries.begin(); tile != tileEntries.end(); ++tile)
            {
                const ModelSpawn &spawn = map_iter->second->UniqueEntries[tile->second];
                if (spawn.flags & MOD_WORLDSPAWN) // WDT spawn, saved as tile 65/65 currently...
                    continue;
                uint32 nSpawns = tileEntries.count(tile->first);
                std::stringstream tilefilename;
                tilefilename.fill('0');
                tilefilename << iDestDir << "/" << std::setw(3) << map_iter->first << "_";
                uint32 x, y;
                StaticMapTree::unpackTileID(tile->first, x, y);
                tilefilename << std::setw(2) << x << "_" << std::setw(2) << y << ".vmtile";
                FILE *tilefile = fopen(tilefilename.str().c_str(), "wb");
                // file header
                if (success && fwrite(VMAP_MAGIC, 1, 8, tilefile) != 8) success = false;
                // write number of tile spawns
                if (success && fwrite(&nSpawns, sizeof(uint32), 1, tilefile) != 1) success = false;
                // write tile spawns
                for (uint32 s=0; s<nSpawns; ++s)
                {
                    if (s)
                        ++tile;
                    const ModelSpawn &spawn2 = map_iter->second->UniqueEntries[tile->second];
                    success = success && ModelSpawn::writeToFile(tilefile, spawn2);
                    // MapTree nodes to update when loading tile:
                    std::map<uint32, uint32>::iterator nIdx = modelNodeIdx.find(spawn2.ID);
                    if (success && fwrite(&nIdx->second, sizeof(uint32), 1, tilefile) != 1) success = false;
                }
                fclose(tilefile);
            }
            // break; //test, extract only first map; TODO: remvoe this line
        }

        // export objects
        std::cout << "\nConverting Model Files" << std::endl;
        for (std::set<std::string>::iterator mfile = spawnedModelFiles.begin(); mfile != spawnedModelFiles.end(); ++mfile)
        {
            std::cout << "Converting " << *mfile << std::endl;
            if (!convertRawFile(*mfile))
            {
                std::cout << "error converting " << *mfile << std::endl;
                success = false;
                break;
            }
        }

        //cleanup:
        for (MapData::iterator map_iter = mapData.begin(); map_iter != mapData.end(); ++map_iter)
        {
            delete map_iter->second;
        }
        return success;
    }

    bool TileAssembler::readMapSpawns()
    {
        std::string fname = iSrcDir + "/dir_bin";
        FILE *dirf = fopen(fname.c_str(), "rb");
        if (!dirf)
        {
            printf("Could not read dir_bin file!\n");
            return false;
        }
        printf("Read coordinate mapping...\n");
        uint32 mapID, tileX, tileY, check=0;
        G3D::Vector3 v1, v2;
        ModelSpawn spawn;
        while (!feof(dirf))
        {
            check = 0;
            // read mapID, tileX, tileY, Flags, adtID, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            check += fread(&mapID, sizeof(uint32), 1, dirf);
            if (check == 0) // EoF...
                break;
            check += fread(&tileX, sizeof(uint32), 1, dirf);
            check += fread(&tileY, sizeof(uint32), 1, dirf);
            if (!ModelSpawn::readFromFile(dirf, spawn))
                break;

            MapSpawns *current;
            MapData::iterator map_iter = mapData.find(mapID);
            if (map_iter == mapData.end())
            {
                printf("spawning Map %d\n", mapID);
                mapData[mapID] = current = new MapSpawns();
            }
            else current = (*map_iter).second;
            current->UniqueEntries.insert(pair<uint32, ModelSpawn>(spawn.ID, spawn));
            current->TileEntries.insert(pair<uint32, uint32>(StaticMapTree::packTileID(tileX, tileY), spawn.ID));
        }
        bool success = (ferror(dirf) == 0);
        fclose(dirf);
        return success;
    }

    bool TileAssembler::calculateTransformedBound(ModelSpawn &spawn)
    {
        std::string modelFilename = iSrcDir + "/" + spawn.name;
        ModelPosition modelPosition;
        modelPosition.iDir = spawn.iRot;
        modelPosition.iScale = spawn.iScale;
        modelPosition.init();

        FILE *rf = fopen(modelFilename.c_str(), "rb");
        if (!rf)
        {
            printf("ERROR: Can't open model file: %s\n", modelFilename.c_str());
            return false;
        }

        AABox modelBound;
        bool boundEmpty=true;
        char ident[8];

        int readOperation = 1;

        // temporary use defines to simplify read/check code (close file and return at fail)
        #define READ_OR_RETURN(V,S) if(fread((V), (S), 1, rf) != 1) { \
                                        fclose(rf); printf("readfail, op = %i\n", readOperation); return(false); }readOperation++;
        #define CMP_OR_RETURN(V,S)  if(strcmp((V),(S)) != 0)        { \
                                        fclose(rf); printf("cmpfail, %s!=%s\n", V, S);return(false); }

        READ_OR_RETURN(&ident, 8);
        CMP_OR_RETURN(ident, "VMAP003");

        // we have to read one int. This is needed during the export and we have to skip it here
        uint32 tempNVectors;
        READ_OR_RETURN(&tempNVectors, sizeof(tempNVectors));

        uint32 groups, wmoRootId;
        char blockId[5];
        blockId[4] = 0;
        int blocksize;
        float *vectorarray = 0;

        READ_OR_RETURN(&groups, sizeof(uint32));
        READ_OR_RETURN(&wmoRootId, sizeof(uint32));
        if (groups != 1) printf("Warning: '%s' does not seem to be a M2 model!\n", modelFilename.c_str());

        for (uint32 g=0; g<groups; ++g) // should be only one for M2 files...
        {
            fseek(rf, 3*sizeof(uint32) + 6*sizeof(float), SEEK_CUR);

            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "GRP ");
            READ_OR_RETURN(&blocksize, sizeof(int));
            fseek(rf, blocksize, SEEK_CUR);

            // ---- indexes
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "INDX");
            READ_OR_RETURN(&blocksize, sizeof(int));
            fseek(rf, blocksize, SEEK_CUR);

            // ---- vectors
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "VERT");
            READ_OR_RETURN(&blocksize, sizeof(int));
            uint32 nvectors;
            READ_OR_RETURN(&nvectors, sizeof(uint32));

            if (nvectors >0)
            {
                vectorarray = new float[nvectors*3];
                READ_OR_RETURN(vectorarray, nvectors*sizeof(float)*3);
            }
            else
            {
                std::cout << "error: model '" << spawn.name << "' has no geometry!" << std::endl;
                return false;
            }

            for (uint32 i=0, indexNo=0; indexNo<nvectors; indexNo++, i+=3)
            {
                Vector3 v = Vector3(vectorarray[i+0], vectorarray[i+1], vectorarray[i+2]);
                v = modelPosition.transform(v);

                if (boundEmpty)
                    modelBound = AABox(v, v), boundEmpty=false;
                else
                    modelBound.merge(v);
            }
            delete[] vectorarray;
            // drop of temporary use defines
            #undef READ_OR_RETURN
            #undef CMP_OR_RETURN
        }
        spawn.iBound = modelBound + spawn.iPos;
        spawn.flags |= MOD_HAS_BOUND;
        fclose(rf);
        return true;
    }

    struct WMOLiquidHeader
    {
        int xverts, yverts, xtiles, ytiles;
        float pos_x;
        float pos_y;
        float pos_z;
        short type;
    };
    //=================================================================
    bool TileAssembler::convertRawFile(const std::string& pModelFilename)
    {
        bool success = true;
        std::string filename = iSrcDir;
        if (filename.length() >0)
            filename.append("/");
        filename.append(pModelFilename);
        FILE *rf = fopen(filename.c_str(), "rb");

        if (!rf)
        {
            printf("ERROR: Can't open model file in form: %s",pModelFilename.c_str());
            printf("...                          or form: %s",filename.c_str() );
            return false;
        }

        char ident[8];

        int readOperation = 1;

        // temporary use defines to simplify read/check code (close file and return at fail)
        #define READ_OR_RETURN(V,S) if(fread((V), (S), 1, rf) != 1) { \
                                        fclose(rf); printf("readfail, op = %i\n", readOperation); return(false); }readOperation++;
        #define CMP_OR_RETURN(V,S)  if(strcmp((V),(S)) != 0)        { \
                                        fclose(rf); printf("cmpfail, %s!=%s\n", V, S);return(false); }

        READ_OR_RETURN(&ident, 8);
        CMP_OR_RETURN(ident, "VMAP003");

        // we have to read one int. This is needed during the export and we have to skip it here
        uint32 tempNVectors;
        READ_OR_RETURN(&tempNVectors, sizeof(tempNVectors));

        uint32 groups;
        uint32 RootWMOID;
        char blockId[5];
        blockId[4] = 0;
        int blocksize;

        READ_OR_RETURN(&groups, sizeof(uint32));
        READ_OR_RETURN(&RootWMOID, sizeof(uint32));

        std::vector<GroupModel> groupsArray;

        for (uint32 g=0; g<groups; ++g)
        {
            std::vector<MeshTriangle> triangles;
            std::vector<Vector3> vertexArray;

            uint32 mogpflags, GroupWMOID;
            READ_OR_RETURN(&mogpflags, sizeof(uint32));
            READ_OR_RETURN(&GroupWMOID, sizeof(uint32));

            float bbox1[3], bbox2[3];
            READ_OR_RETURN(bbox1, sizeof(float)*3);
            READ_OR_RETURN(bbox2, sizeof(float)*3);

            uint32 liquidflags;
            READ_OR_RETURN(&liquidflags, sizeof(uint32));

            // will this ever be used? what is it good for anyway??
            uint32 branches;
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "GRP ");
            READ_OR_RETURN(&blocksize, sizeof(int));
            READ_OR_RETURN(&branches, sizeof(uint32));
            for (uint32 b=0; b<branches; ++b)
            {
                uint32 indexes;
                // indexes for each branch (not used jet)
                READ_OR_RETURN(&indexes, sizeof(uint32));
            }

            // ---- indexes
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "INDX");
            READ_OR_RETURN(&blocksize, sizeof(int));
            uint32 nindexes;
            READ_OR_RETURN(&nindexes, sizeof(uint32));
            if (nindexes >0)
            {
                uint16 *indexarray = new uint16[nindexes];
                READ_OR_RETURN(indexarray, nindexes*sizeof(uint16));
                for (uint32 i=0; i<nindexes; i+=3)
                {
                    triangles.push_back(MeshTriangle(indexarray[i], indexarray[i+1], indexarray[i+2]));
                }
                delete[] indexarray;
            }

            // ---- vectors
            READ_OR_RETURN(&blockId, 4);
            CMP_OR_RETURN(blockId, "VERT");
            READ_OR_RETURN(&blocksize, sizeof(int));
            uint32 nvectors;
            READ_OR_RETURN(&nvectors, sizeof(uint32));

            if (nvectors >0)
            {
                float *vectorarray = new float[nvectors*3];
                READ_OR_RETURN(vectorarray, nvectors*sizeof(float)*3);
                for (uint32 i=0; i<nvectors; ++i)
                {
                    vertexArray.push_back( Vector3(vectorarray + 3*i) );
                }
                delete[] vectorarray;
            }
            // ----- liquid
            WmoLiquid *liquid = 0;
            if (liquidflags& 1)
            {
                WMOLiquidHeader hlq;
                READ_OR_RETURN(&blockId, 4);
                CMP_OR_RETURN(blockId, "LIQU");
                READ_OR_RETURN(&blocksize, sizeof(int));
                READ_OR_RETURN(&hlq, sizeof(WMOLiquidHeader));
                liquid = new WmoLiquid(hlq.xtiles, hlq.ytiles, Vector3(hlq.pos_x, hlq.pos_y, hlq.pos_z), hlq.type);
                uint32 size = hlq.xverts*hlq.yverts;
                READ_OR_RETURN(liquid->GetHeightStorage(), size*sizeof(float));
                size = hlq.xtiles*hlq.ytiles;
                READ_OR_RETURN(liquid->GetFlagsStorage(), size);
            }

            groupsArray.push_back(GroupModel(mogpflags, GroupWMOID, AABox(Vector3(bbox1), Vector3(bbox2))));
            groupsArray.back().setMeshData(vertexArray, triangles);
            groupsArray.back().setLiquidData(liquid);

            // drop of temporary use defines
            #undef READ_OR_RETURN
            #undef CMP_OR_RETURN

        }
        fclose(rf);

        // write WorldModel
        WorldModel model;
        model.setRootWmoID(RootWMOID);
        if (groupsArray.size())
        {
            model.setGroupModels(groupsArray);
            success = model.writeFile(iDestDir + "/" + pModelFilename + ".vmo");
        }

        //std::cout << "readRawFile2: '" << pModelFilename << "' tris: " << nElements << " nodes: " << nNodes << std::endl;
        return success;
    }
}
