#include <Foundation/Containers/Implementation/BitIterator.h>

/// Helper base class to iterate over the bit indices or bit values of an integer.
/// \tparam DataType The type of data that is being iterated over.
/// \tparam ReturnsIndex If set, returns the index of the bit. Otherwise returns the value of the bit, i.e. NS_BIT(value).
/// \tparam ReturnType Returned value type of the iterator.
/// \sa nsIterateBitValues, nsIterateBitIndices
template <typename DataType, bool ReturnsIndex, typename ReturnType = DataType>
struct nsIterateBits
{
  explicit nsIterateBits(DataType data)
  {
    m_Data = data;
  }

  nsBitIterator<DataType, ReturnsIndex, ReturnType> begin() const
  {
    return nsBitIterator<DataType, ReturnsIndex, ReturnType>(m_Data);
  };

  nsBitIterator<DataType, ReturnsIndex, ReturnType> end() const
  {
    return nsBitIterator<DataType, ReturnsIndex, ReturnType>();
  };

  DataType m_Data = {};
};

/// \brief Helper class to iterate over the bit values of an integer.
/// The class can iterate over the bits of any unsigned integer type that is equal to or smaller than nsUInt64.
/// \code{.cpp}
///    nsUInt64 bits = 0b1101;
///    for (auto bit : nsIterateBitValues(bits))
///    {
///      nsLog::Info("{}", bit); // Outputs 1, 4, 8
///    }
/// \endcode
/// \tparam DataType The type of data that is being iterated over.
/// \tparam ReturnType Returned value type of the iterator. Defaults to same as DataType.
template <typename DataType, typename ReturnType = DataType>
struct nsIterateBitValues : public nsIterateBits<DataType, false, ReturnType>
{
  explicit nsIterateBitValues(DataType data)
    : nsIterateBits<DataType, false, ReturnType>(data)
  {
  }
};

/// \brief Helper class to iterate over the bit indices of an integer.
/// The class can iterate over the bits of any unsigned integer type that is equal to or smaller than nsUInt64.
/// \code{.cpp}
///    nsUInt64 bits = 0b1101;
///    for (auto bit : nsIterateBitIndices(bits))
///    {
///      nsLog::Info("{}", bit); // Outputs 0, 2, 3
///    }
/// \endcode
/// \tparam DataType The type of data that is being iterated over.
/// \tparam ReturnType Returned value type of the iterator. Defaults to same as DataType.
template <typename DataType, typename ReturnType = DataType>
struct nsIterateBitIndices : public nsIterateBits<DataType, true, ReturnType>
{
  explicit nsIterateBitIndices(DataType data)
    : nsIterateBits<DataType, true, ReturnType>(data)
  {
  }
};