#pragma once

#include <common/Singleton.h>

namespace blue_sky
{

/**
 * 複数の Scene を管理する Manager
 *
 */
class SceneManager : public common::Singleton< SceneManager >
{
public:

private:

public:
	SceneManager() { }
	~SceneManager() { }
};

} // namespace blue_sky
