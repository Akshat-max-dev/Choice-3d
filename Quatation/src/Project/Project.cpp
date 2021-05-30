#include "Project.h"

#include "Error.h"

namespace choice
{
	Project::Project(const std::string& name, const std::string& path)
		:mName(name), mDirectory(path)
	{
		//To Check If Already Exists
		uint32_t counter = 0;
		while (ghc::filesystem::exists(mDirectory + "\\" + mName + "\\" + mName + ".cproj"))
		{
			counter++;
			std::string msg = "Project Already Exists. Creating " + mName + std::to_string(counter);
			Message<WARNING>(msg.c_str(), MESSAGE_ORIGIN::FILESYSTEM);
			mName += std::to_string(counter);
		}

		ghc::filesystem::create_directory(mDirectory + "\\" + mName);

		mActiveScene = new Scene("DefaultScene", mDirectory + "\\" + mName);
	}

	Project::Project(const std::string& cproj)
	{
		std::string temp = cproj.substr(cproj.find_last_of('\\') + 1, cproj.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = cproj.substr(0, cproj.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);

		std::ifstream readcproj(cproj, std::ios::in);
		if (readcproj.fail())
		{
			std::string msg = "Failed To Open " + cproj;
			Message<ERROR_MSG>(msg.c_str(), MESSAGE_ORIGIN::FILESYSTEM);
			return;
		}

		std::string activescenename;
		readcproj >> activescenename;

		mActiveScene = new Scene(mDirectory + "\\" + mName + "\\" + activescenename + "\\" + activescenename + ".cscene");
		readcproj.close();
	}

	Project::~Project()
	{
		if (!ghc::filesystem::exists(mDirectory + "\\" + mName + "\\" + mName + ".cproj"))
			Save();

		delete mActiveScene;
	}

	void Project::Save()
	{
		std::ofstream cproj(mDirectory + "\\" + mName + "\\" + mName + ".cproj", std::ios::out);
		if (cproj.fail())
		{
			Message<ERROR_MSG>("Failed To Save Project!", MESSAGE_ORIGIN::FILESYSTEM);
			cproj.close(); 
			return;
		}
		cproj << mActiveScene->Name();
		cproj.close();

		mActiveScene->Save();
	}

}