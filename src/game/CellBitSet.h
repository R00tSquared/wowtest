#ifndef HELLGROUND_CELLBITSET_H
#define HELLGROUND_CELLBITSET_H

class CellBitSet
{	// store fixed-length sequence of Boolean elements
public:

    CellBitSet()
    {	// construct with all false values
        // max bit that can be addressed from Map.cpp is 1024 * 1024 (last is excluded in reset())
        setStartEndForReset(0, TOTAL_NUMBER_OF_CELLS_PER_MAP * TOTAL_NUMBER_OF_CELLS_PER_MAP);
        reset();
    }

    void setTrue(uint64 _Pos)
    {
        _Array[_Pos / _Bitsperword] |= (uint64)1 << _Pos % _Bitsperword;
    }

    void reset()
    {
        for (int64 _Wpos = _word_end - 1; _word_start <= _Wpos; --_Wpos)
            _Array[_Wpos] = 0;

        /*bool test = findNotReset();
        if (!test)
        {
            test = !test;
        }*/
    }

    /*bool findNotReset() 
    {
        for (int64 _Wpos = 0; _Wpos < 16384; ++_Wpos)
        {
            if (_Array[_Wpos])
                return false;
        }
        return true;
    }*/

    bool testAt(uint64 _Pos) const
    {	// test if bit at _Pos is set
        return ((_Array[_Pos / _Bitsperword]
            & ((uint64)1 << _Pos % _Bitsperword)) != 0);
    }

    void setStartEndForReset(uint64 bit_start, uint64 bit_end)
    {
        _word_start = bit_start / _Bitsperword;
        _word_end = bit_end / _Bitsperword;
    }

private:

    enum : uint64
    {	// parameters for packing bits into words
        _Bitsperword = (uint64)(CHAR_BIT * sizeof(uint64)),
        _Words = (uint64)((TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP) / _Bitsperword)
    };	// NB: number of words - 1

    uint64 _Array[_Words];	// the set of bits
    int64 _word_start;
    int64 _word_end;
};

#endif
