#include "Project.h"

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
			std::cout << "Project Already Exists. Creating " + mName + std::to_string(counter) << std::endl;
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
			std::cout << "Cannot Open Project" << std::endl;
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
			std::cout << "Error Saving Project" << std::endl;
			cproj.close(); 
			return;
		}
		cproj << mActiveScene->Name();
		cproj.close();

		mActiveScene->Save();
	}

}