#ifndef MD_ARRAY_ACCESS_HPP
#define MD_ARRAY_ACCESS_HPP



/// Template parameters
/// N1: Number of array dimensions
/// N2: current dimension
template <typename RETURN_TYPE, 
					class ID_TYPE,
					class ARRAY_TYPE,
					unsigned N1,
					unsigned N2 = N1>
class smoc_md_array_access 
	: public smoc_md_array_access<RETURN_TYPE,ID_TYPE,ARRAY_TYPE,N1,N2-1>
{
public:
	typedef smoc_md_array_access<RETURN_TYPE,ID_TYPE,ARRAY_TYPE,N1,N2-1> base_type;
	typedef unsigned long size_t;
public:
	
	smoc_md_array_access(ARRAY_TYPE& array)
		: base_type(array) {}

public:
	//NOT virtual!
	base_type& operator[](size_t idx) {
		(*this).id[N1-N2] = idx;
		return (*this);
	}

	const base_type& operator[](size_t idx) const {
		(*this).id[N1-N2] = idx;
		return (*this);
	}
};

template <typename RETURN_TYPE,
					class ID_TYPE,
					class ARRAY_TYPE,
					unsigned N1>
class smoc_md_array_access<RETURN_TYPE,ID_TYPE,ARRAY_TYPE,N1,1>
{
public:
	smoc_md_array_access(ARRAY_TYPE& array)
		: id(N1),array(array){}
public:
	typedef unsigned long size_t;
public:
	//NOT virtual!
	RETURN_TYPE operator[](size_t idx) {
		id[N1-1] = idx;
		return array[id];
	}

	const RETURN_TYPE operator[](size_t idx) const {
		id[N1-1] = idx;
		return array[id];
	}

protected:
	mutable ID_TYPE id;
private:
	ARRAY_TYPE &array;

};


#endif
