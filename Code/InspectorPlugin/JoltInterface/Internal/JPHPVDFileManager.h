/*
 *   Copyright (c) 2024-present Mikael K. Aboagye & WD Studios L.L.C.
 *   All rights reserved.
 *   This Project & Code is Licensed under the MIT License.
 */

/*
*   JPHPVDFileManager.h
*   
*   JPHPVDFileManager is responsible for managing and writing data to a file.
*   It provides functions to write body, character, constraint, and soft body shape data.
*/

#pragma once
#include <InspectorPlugin/InspectorPluginDLL.h>
#include <Foundation/Math/Math.h>
#include <Jolt/Jolt.h>

class nsOSFile;
class nsAbstractObjectGraph;

namespace JPH
{
    class Body;
    class Character;
    class Constraint;
    class SoftBodyShape;
}

namespace JDebug::API::IO
{
    class NS_INSPECTORPLUGIN_DLL JPHPVDFileManager
    {
        public:
            /**
             * @brief Default constructor.
             */
            JPHPVDFileManager() = default;

            /**
             * @brief Constructor.
             * @param in_file The file to manage and write data to.
             */
            explicit JPHPVDFileManager(const nsOSFile& in_file);

            /**
             * @brief Destructor.
             */
            virtual ~JPHPVDFileManager();

            /**
             * @brief Writes body data to the file.
             * @param in_data The body data to write.
             */
            void WriteBodyData(const JPH::Body& in_data);
            
            /**
             * @brief Writes character data to the file.
             * @param in_data The character data to write.
             */
            void WriteCharacterData(const JPH::Character& in_data);

            /**
             * @brief Writes constraint data to the file.
             * @param in_data The constraint data to write.
             */
            void WriteConstraintData(const JPH::Constraint& in_data);

            /**
             * @brief Writes soft body shape data to the file.
             * @param in_data The soft body shape data to write.
             */
            void WriteSoftBodyShapeData(const JPH::SoftBodyShape& in_data);
        private:
            nsAbstractObjectGraph* m_pGraph; ///< The object graph. this is used to write data to the file.
    };
}