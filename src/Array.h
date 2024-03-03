#pragma once

template<class T>
class Array
{
private:
	int* counter = nullptr;
	T* data = nullptr;
	size_t size = 0;

	void destroyReference()
	{
		if (counter != nullptr) // if this is not an empty array
		{
			if (--(*counter) <= 0)
			{
				//std::cout << "all references lost, deleting... " << data << '\n';
				delete[] data;
				delete counter;
				data = nullptr;
				counter = nullptr;
				size = 0;
			}
		}
	}

public:
	template<class... TArgs>
	Array(TArgs... args)
	{
		size = sizeof...(args);
		data = new T[size]{ args... };
		counter = new int(1);
	}

	static Array ofSize(size_t size, T inital)
	{
		auto arr = Array();

		arr.size = size;
		arr.data = new T[size];
		arr.counter = new int(1);

		for (auto& item : arr)
			item = inital;

		return arr;
	}

	/// <summary>
	/// Creates an empty array identity;
	/// </summary>
	Array()
	{
		size = 0;
		data = nullptr;
		counter = nullptr;
	}

	~Array()
	{
		destroyReference();
	}

	// Copy assignment
	Array& operator= (const Array& other)
	{
		destroyReference();
		data = other.data;
		size = other.size;
		counter = other.counter;
		(*counter)++;
		return *this;
	}

	Array(const Array& other) 
	{ 
		data = other.data;
		size = other.size;
		counter = other.counter;
		(*counter)++;
	}

	int length() const { return size; }

	bool isEmpty() const { return size <= 0; }

	T& operator[](size_t index) { return data[index]; }

	const T& operator[](size_t index) const { return data[index]; }

	T* begin() { return &data[0]; }

	const T* begin() const { return &data[0]; }

	T* end() { return &data[size]; }

	const T* end() const { return &data[size]; }
};

template<class T, class... TArgs>
Array<T> arrayOf(TArgs... args) { return Array<T>(args...); }
