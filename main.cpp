#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <cstdint>
#include <filesystem>

struct Bone
{
	std::string name;
	int32_t parentId = -1;
};

struct Vector3
{
	float x = 0.f, y = 0.f, z = 0.f;
};

struct Quaternion
{
	int16_t x = 0, y = 0, z = 0, w = -32768; // fixed point
};

struct Frame
{
	std::vector<Vector3> transforms;
	std::vector<Quaternion> quaternions;
};

struct Model
{
	uint32_t version = 8;
	uint32_t unknown0 = 1;
	float frameRate = 20.f;
	std::string skeletonName;
	uint32_t flagCount = 0;
	float animationTotalPlayTimeInSeconds = 0.f;
	uint32_t boneCount = 0;
	std::vector<Bone> bones;
	uint32_t unknown1 = 6;
	uint32_t unknown2 = 1;
	uint32_t transformCount = 0;
	uint32_t quaternionCount = 0;
	uint32_t frameCount = 0;
	std::vector<Frame> frames;
};

class Deserializer
{
public:
	Deserializer(std::istream& in) : in(in) {}

	template<typename T>
	Deserializer& read(T& object, size_t size = sizeof(T))
	{
		if (!this->good()) return *this;

		this->in.read(reinterpret_cast<char*>(&object), size);

		return *this;
	}

	bool good() const
	{
		return *this;
	}

	operator bool() const
	{
		return !!this->in;
	}

private:
	std::istream& in;
};

template<typename T>
Deserializer& operator>>(Deserializer& d, T& object)
{
	static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type to use this overload");

	d.read(object);

	return d;
}

template<typename T>
Deserializer& operator>>(Deserializer& d, std::vector<T>& object)
{
	for (auto&& i : object)
		d >> i;

	return d;
}

Deserializer& operator>>(Deserializer& d, std::string& object)
{
	uint16_t len = 0; // file uses 16-bit integer to store string length
	d >> len;

	object.resize(len);
	d.read(*object.data(), size_t(len));

	return d;
}

Deserializer& operator>>(Deserializer& d, Bone& object)
{
	d >> object.name;
	d >> object.parentId;
	return d;
}

Deserializer& operator>>(Deserializer& d, Vector3& object)
{
	d >> object.x >> object.y >> object.z;
	return d;
}

Deserializer& operator>>(Deserializer& d, Quaternion& object)
{
	d >> object.x >> object.y >> object.z >> object.w;
	return d;
}

Deserializer& operator>>(Deserializer& d, Model& object)
{
	// Header
	d >> object.version >> object.unknown0
		>> object.frameRate >> object.skeletonName
		>> object.flagCount >> object.animationTotalPlayTimeInSeconds;

	// Bones
	d >> object.boneCount;
	object.bones.resize(object.boneCount);
	d >> object.bones;


	// Unknown zone that looks like padding
	d >> object.unknown1 >> object.unknown2;

	uint8_t lmaoPadding;

	// Handle all the 0x0C bytes
	for (size_t i = 0; i < object.bones.size(); i++)
		d >> lmaoPadding;

	// Handle all the 0x08 bytes
	for (size_t i = 0; i < object.bones.size(); i++)
		d >> lmaoPadding;

	// Handle all the 0x00 bytes
	for (size_t i = 0; i < 16; i++)
		d >> lmaoPadding;

	// Frame stuff?
	d >> object.transformCount >> object.quaternionCount >> object.frameCount;

	object.frames.resize(object.frameCount);
	for (size_t i = 0; i < object.frames.size(); i++)
	{
		auto&& [transforms, quaternions] = object.frames.at(i);
		transforms.resize(object.quaternionCount);
		quaternions.resize(object.transformCount);

		for (size_t j = 0; j < transforms.size(); j++)
		{
			d >> transforms.at(j);
		}

		for (size_t j = 0; j < quaternions.size(); j++)
		{
			d >> quaternions.at(j);
		}
	}

	return d;
}

class Serializer
{
public:
	Serializer(std::ostream& out) : out(out) {}

	template<typename T>
	Serializer& write(const T& object, size_t size = sizeof(T))
	{
		if (!this->good()) return *this;

		this->out.write(reinterpret_cast<const char*>(&object), size);

		return *this;
	}

	bool good() const
	{
		return *this;
	}

	operator bool() const
	{
		return !!this->out;
	}

private:
	std::ostream& out;
};

template<typename T>
Serializer& operator<<(Serializer& s, const T& object)
{
	static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type to use this overload");

	s.write(object);

	return s;
}

template<typename T>
Serializer& operator<<(Serializer& s, const std::vector<T>& object)
{
	for (auto&& i : object)
		s << i;

	return s;
}

Serializer& operator<<(Serializer& s, const std::string& object)
{
	size_t len = object.size();

	s << uint16_t(len); // file uses 16-bit integer to store string length

	s.write(*object.data(), len);

	return s;
}

Serializer& operator<<(Serializer& s, const Bone& object)
{
	s << object.name;
	s << object.parentId;
	return s;
}

Serializer& operator<<(Serializer& s, const Vector3& object)
{
	s << object.x << object.y << object.z;
	return s;
}

Serializer& operator<<(Serializer& s, const Quaternion& object)
{
	s << object.x << object.y << object.z << object.w;
	return s;
}

Serializer& operator<<(Serializer& s, const Model& object)
{
	// Header
	s << object.version << object.unknown0
		<< object.frameRate << object.skeletonName
		<< object.flagCount << object.animationTotalPlayTimeInSeconds;

	// Bones
	s << object.boneCount;
	s << object.bones;


	// Unknown zone that looks like padding
	s << object.unknown1 << object.unknown2;

	// Handle all the 0x0C bytes
	for (size_t i = 0; i < object.bones.size(); i++)
		s << '\x0c';

	// Handle all the 0x08 bytes
	for (size_t i = 0; i < object.bones.size(); i++)
		s << '\x08';

	// Handle all the 0x00 bytes
	for (size_t i = 0; i < 16; i++)
		s << '\x00';

	// Frame stuff?
	s << object.transformCount << object.quaternionCount << object.frameCount;

	for (size_t i = 0; i < object.frames.size(); i++)
	{
		auto&& [transforms, quaternions] = object.frames.at(i);

		for (size_t j = 0; j < transforms.size(); j++)
		{
			s << transforms.at(j);
		}

		for (size_t j = 0; j < quaternions.size(); j++)
		{
			s << quaternions.at(j);
		}
	}

	return s;
}

std::ostream& outputModel(std::ostream& out, const Model& model)
{
	out << model.version << '\n';
	out << model.unknown0 << '\n';
	out << model.frameRate << '\n';
	out << model.skeletonName << '\n';
	out << model.flagCount << '\n';
	out << model.animationTotalPlayTimeInSeconds << '\n';

	out << model.boneCount << '\n';
	for (size_t i = 0; i < model.bones.size(); i++)
	{
		auto&& [name, parentId] = model.bones.at(i);
		out << name << ' ' << i << ' ' << parentId << '\n';
	}

	out << model.transformCount << '\n';
	out << model.quaternionCount << '\n';
	out << model.frameCount << '\n';

	for (size_t i = 0; i < model.frames.size(); i++)
	{
		auto&& [transforms, quaternions] = model.frames.at(i);

		for (size_t j = 0; j < transforms.size(); j++)
		{
			auto&& [x, y, z] = transforms.at(j);
			out << x << ' ' << y << ' ' << z << '\n';
		}

		for (size_t j = 0; j < quaternions.size(); j++)
		{
			auto&& [xi, yi, zi, wi] = quaternions.at(j);

			float x = float(xi) / 32767.f;
			float y = float(yi) / -32767.f;
			float z = float(zi) / -32767.f;
			float w = float(wi) / 32767.f;

			out << x << ' ' << y << ' ' << z << ' ' << w << '\n';
		}
	}

	return out;
}

std::istream& inputModel(std::istream& in, Model& model)
{
	in >> model.version;
	in >> model.unknown0;
	in >> model.frameRate;
	in >> model.skeletonName;
	in >> model.flagCount;
	in >> model.animationTotalPlayTimeInSeconds;

	in >> model.boneCount;
	model.bones.resize(model.boneCount);
	for (size_t i = 0; i < model.bones.size(); i++)
	{
		auto&& [name, parentId] = model.bones.at(i);
		in >> name >> i >> parentId;
	}

	in >> model.transformCount;
	in >> model.quaternionCount;
	in >> model.frameCount;

	model.frames.resize(model.frameCount);
	for (size_t i = 0; i < model.frames.size(); i++)
	{
		auto&& [transforms, quaternions] = model.frames.at(i);

		transforms.resize(model.transformCount);
		quaternions.resize(model.quaternionCount);

		for (size_t j = 0; j < transforms.size(); j++)
		{
			auto&& [x, y, z] = transforms.at(j);
			in >> x >> y >> z;
		}

		for (size_t j = 0; j < quaternions.size(); j++)
		{
			auto&& [xi, yi, zi, wi] = quaternions.at(j);
			float x = 0.f, y = 0.f, z = 0.f, w = 0.f;

			in >> x >> y >> z >> w;

			xi = int16_t(x * 32767.f);
			yi = int16_t(y * 32767.f);
			zi = int16_t(z * 32767.f);
			wi = int16_t(w * 32767.f);
		}
	}

	return in;
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
	{
		std::cout << "Please specify an input file.\n";
		std::system("pause");
		return 1;
	}

	bool silent = false;

	if (argc == 3)
	{
		if(std::string(argv[2]) == "-s") silent = true;
		else
		{
			std::cout << "Too many arguments. If you meant to run in silent mode, use the -s flag.\n";
			std::system("pause");
			return 1;
		}
	}

	std::filesystem::path inPath(argv[1]);
	std::string extension = inPath.extension().string();


	if (extension == ".anim")
	{
		auto outPath = inPath;
		outPath.replace_extension(".txt");
		std::ifstream in(inPath, std::ios::binary);
		std::ofstream out(outPath);

		if (!in)
		{
			std::cout << "Input file couldn't be opened. Try again.\n";
			if(!silent) std::system("pause");
			return 1;
		}

		if (!out)
		{
			std::cout << "Output file couldn't be opened. Try again.\n";
			if (!silent) std::system("pause");
			return 1;
		}

		Deserializer des(in);
		Model model;
		des >> model;

		outputModel(out, model);
	}
	else if (extension == ".txt")
	{
		auto outPath = inPath;
		outPath.replace_extension(".anim");
		std::ifstream in(inPath);
		std::ofstream out(outPath, std::ios::binary);

		if (!in)
		{
			std::cout << "Input file couldn't be opened. Try again.\n";
			if (!silent) std::system("pause");
			return 1;
		}

		if (!out)
		{
			std::cout << "Output file couldn't be opened. Try again.\n";
			if (!silent) std::system("pause");
			return 1;
		}

		Serializer s(out);
		Model model;
		inputModel(in, model);

		s << model;
	}
	else
	{
		std::cout << "Extension of input file must be .txt or .anim\n";
		if (!silent) std::system("pause");
		return 1;
	}
}
