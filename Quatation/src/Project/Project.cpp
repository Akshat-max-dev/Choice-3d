#include "Project.h"

#include "BinaryHelper.h"

namespace choice
{
	Project::Project(const std::string& name, const std::string& path)
		:mName(name), mDirectory(path)
	{
		//To Check If Already Exists
		std::ifstream check(mDirectory + "\\" + mName + "\\" + mName + ".cproj", std::ios::in | std::ios::binary);
		if (check.is_open())
		{
			std::cout << "Project Already Exists. Creating " + mName + "1" << std::endl;
			mName += "1";
		}
		check.close();

		ghc::filesystem::create_directory(mDirectory + "\\" + mName);

		mActiveScene = new Scene("Default Scene", mDirectory + "\\" + mName);
	}

	Project::Project(const std::string& cproj)
	{
		std::string temp = cproj.substr(cproj.find_last_of('\\') + 1, cproj.size());
		mName = temp.substr(0, temp.find_last_of('.'));
		temp = cproj.substr(0, cproj.find_last_of('\\'));
		mDirectory = temp.substr(0, temp.size() - mName.size() - 1);

		std::ifstream readcproj(cproj, std::ios::in | std::ios::binary);
		if (readcproj.fail())
		{
			std::cout << "Cannot Open Project" << std::endl;
			return;
		}

		std::string activescenename;
		Binary::Read<std::string>(readcproj, activescenename);

		mActiveScene = new Scene(mDirectory + "\\" + mName + "\\" + activescenename + "\\" + activescenename + ".cscene");
		readcproj.close();
	}

	Project::~Project()
	{
		std::ifstream open(mDirectory + "\\" + mName + "\\" + mName + ".cproj", std::ios::in | std::ios::binary);
		if (!open.is_open())
		{
			Save();
		}
		else
		{
			mActiveScene->Clean();
		}
		open.close();
		delete mActiveScene;
	}

	void Project::Save()
	{
		std::ofstream cproj(mDirectory + "\\" + mName + "\\" + mName + ".cproj", std::ios::out | std::ios::binary);
		if (cproj.fail())
		{
			std::cout << "Error Saving Project" << std::endl;
			cproj.close(); 
			return;
		}
		Binary::Write<std::string>(cproj, mActiveScene->Name());
		cproj.close();

		mActiveScene->Save();
	}

}