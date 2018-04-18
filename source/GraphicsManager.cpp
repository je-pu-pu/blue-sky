#include "GraphicsManager.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"
#include "FbxFileLoader.h"

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem.hpp>

#include <iostream>

namespace blue_sky
{

GraphicsManager::GraphicsManager()
{
	
}

GraphicsManager::~GraphicsManager()
{
	cleanup_loader();
}

void GraphicsManager::setup_loader()
{
	fbx_file_loader_ = new FbxFileLoader();
}

void GraphicsManager::cleanup_loader()
{
	fbx_file_loader_.release();	
}

/**
 * ��`�������b�V����ǂݍ���
 *
 * @param name ���O
 * @param skinning_animation_set �����ɓǂݍ��ރX�L�j���O�A�j���[�V���������i�[����|�C���^
 * @return ��`�������b�V���A�܂��͎��s���� 0 ��Ԃ�
 */
DrawingMesh* GraphicsManager::load_drawing_mesh( const char_t* name, common::safe_ptr< SkinningAnimationSet >& skinning_animation_set )
{
	std::string file_path = string_t( "media/model/" ) + name;

	std::cout << "--- loading " << name << " ---" << std::endl;
	common::timer t;

	DrawingMesh* mesh = create_drawing_mesh();
	
	bool loaded = false;

	if ( ! loaded && boost::filesystem::exists( file_path + ".bin.fbx" ) )
	{
		if ( ! boost::filesystem::exists( file_path + ".fbx" ) || boost::filesystem::last_write_time( file_path + ".fbx" ) < boost::filesystem::last_write_time( file_path + ".bin.fbx" ) )
		{
			loaded = mesh->load_fbx( ( file_path + ".bin.fbx" ).c_str(), fbx_file_loader_.get(), skinning_animation_set );
		}
	}

	/*
	if ( ! loaded && boost::filesystem::exists( file_path  + ".fbx" ) )
	{
		loaded = mesh->load_fbx( ( file_path  + ".fbx" ).c_str(), fbx_file_loader_.get(), skinning_animation_set );

		if ( loaded )
		{
			fbx_file_loader_->save( ( file_path + ".bin.fbx" ).c_str() );
		}
	}
	*/

	if ( ! loaded && boost::filesystem::exists( file_path + ".obj" ) )
	{
		loaded = mesh->load_obj( ( file_path + ".obj" ).c_str() );
	}

	std::cout << "--- loaded " << name << " : " << t.elapsed() << "---" << std::endl;

	return mesh;
}

/**
 * ��`�������C����ǂݍ���
 *
 * @param name ���O
 * @return ��`�������C���A�܂��͎��s���� 0 ��Ԃ�
 */
DrawingLine* GraphicsManager::load_drawing_line( const char_t* name )
{
	string_t file_path = string_t( "media/model/" ) + name + "-line.obj";

	if ( ! boost::filesystem::exists( file_path ) )
	{
		return 0;
	}

	DrawingLine* line = create_drawing_line();
	line->load_obj( file_path.c_str() );

	return line;
}

}