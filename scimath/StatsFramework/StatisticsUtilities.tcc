//# Copyright (C) 2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#ifndef SCIMATH_STATISTICSUTILITIES_TCC
#define SCIMATH_STATISTICSUTILITIES_TCC

#include <casacore/scimath/StatsFramework/StatisticsUtilities.h>

#include <casacore/casa/OS/OMP.h>
#include <casacore/casa/Utilities/GenSort.h>
#include <casacore/casa/Utilities/PtrHolder.h>
#include <casacore/scimath/StatsFramework/ClassicalStatisticsData.h>

#include <iostream>

namespace casacore {

template <class AccumType>
const AccumType StatisticsUtilities<AccumType>::TWO = AccumType(2);

// For performance reasons, we ensure code is inlined rather than
// calling other functions. The performance
// benefits become important for very large datasets

#define _NLINEAR \
	npts++; \
	sum += datum; \
	mean += (datum - mean)/npts;

#define _WLINEAR \
	npts++; \
	sumweights += weight; \
	wsum += weight*datum; \
	wmean += weight/sumweights*(datum - wmean);

#define _NQUAD \
	sumsq += datum*datum; \
	AccumType prevMean = mean; \
	_NLINEAR \
	nvariance += (datum - prevMean)*(datum - mean);

#define _WQUAD \
	wsumsq += weight*datum*datum; \
	AccumType prevMean = wmean; \
	_WLINEAR \
	wnvariance += weight*(datum - prevMean)*(datum - wmean);

#define _MAXMIN \
	if (npts == 1) { \
		datamax = datum; \
		maxpos = location; \
		datamin = datum; \
		minpos = location; \
	} \
	else if (datum > datamax) { \
		datamax = datum; \
		maxpos = location; \
	} \
	else if (datum < datamin) { \
		datamin = datum; \
		minpos = location; \
	}

template <class AccumType> void StatisticsUtilities<AccumType>::accumulate (
	Double& npts, AccumType& sum, AccumType& mean, const AccumType& datum
) {
	_NLINEAR
}

template <class AccumType> void StatisticsUtilities<AccumType>::waccumulate (
	Double& npts, AccumType& sumweights, AccumType& wsum, AccumType& wmean,
	const AccumType& datum, const AccumType& weight
) {
	_WLINEAR
}

template <class AccumType> void StatisticsUtilities<AccumType>::accumulate (
	Double& npts, AccumType& sum, AccumType& mean, AccumType& nvariance,
	AccumType& sumsq, const AccumType& datum
) {
	_NQUAD
}

template <class AccumType> void StatisticsUtilities<AccumType>::waccumulate (
	Double& npts, AccumType& sumweights, AccumType& wsum,
	AccumType& wmean, AccumType& wnvariance, AccumType& wsumsq,
	const AccumType& datum, const AccumType& weight
) {
	_WQUAD
}

template <class AccumType> template <class LocationType>
void StatisticsUtilities<AccumType>::accumulate (
	Double& npts, AccumType& sum, AccumType& mean, AccumType& nvariance,
	AccumType& sumsq, AccumType& datamin,
	AccumType& datamax, LocationType& minpos, LocationType& maxpos,
	const AccumType& datum, const LocationType& location
) {
	_NQUAD
	_MAXMIN
}

template <class AccumType> template <class LocationType, class DataType>
void StatisticsUtilities<AccumType>::accumulate (
    Double& npts, AccumType& sum, AccumType& mean, AccumType& nvariance,
    AccumType& sumsq, DataType& datamin,
    DataType& datamax, LocationType& minpos, LocationType& maxpos,
    const DataType& datum, const LocationType& location
) {
    _NQUAD
    _MAXMIN
}

template <class AccumType> template <class LocationType>
void StatisticsUtilities<AccumType>::waccumulate (
	Double& npts, AccumType& sumweights, AccumType& wsum, AccumType& wmean, AccumType& wnvariance,
	AccumType& wsumsq, AccumType& datamin, AccumType& datamax,
	LocationType& minpos, LocationType& maxpos,
	const AccumType& datum, const AccumType& weight, const LocationType& location
) {
	_WQUAD
	_MAXMIN
}

template <class AccumType> template <class LocationType>
Bool StatisticsUtilities<AccumType>::doMax(
	AccumType& datamax, LocationType& maxpos, Bool isFirst,
	const AccumType& datum, const LocationType& location
) {
	if (isFirst || datum > datamax) {
		datamax = datum;
		maxpos = location;
		return True;
	}
	return False;
}

template <class AccumType> template <class LocationType>
Bool StatisticsUtilities<AccumType>::doMin(
	AccumType& datamin, LocationType& minpos, Bool isFirst,
	const AccumType& datum, const LocationType& location
) {
	if (isFirst || datum < datamin) {
		datamin = datum;
		minpos = location;
		return True;
	}
	return False;
}

#define _NQUADSYM \
	npts += 2; \
	AccumType reflect = TWO*center - datum; \
	sumsq += datum*datum + reflect*reflect; \
	AccumType diff = datum - center; \
	nvariance += TWO*diff*diff;

#define _WQUADSYM \
	npts += 2; \
	sumweights += TWO*weight; \
	AccumType reflect = TWO*center - datum; \
	wsumsq += weight*(datum*datum + reflect*reflect); \
	AccumType diff = datum - center; \
	wnvariance += TWO*weight*diff*diff;

#define _MAXMINSYM \
	if (npts == 2) { \
		datamax = datum; \
		maxpos = location; \
		datamin = datum; \
		minpos = location; \
	} \
	else if (datum > datamax) { \
		datamax = datum; \
		maxpos = location; \
	} \
	else if (datum < datamin) { \
		datamin = datum; \
		minpos = location; \
	}

template <class AccumType> void StatisticsUtilities<AccumType>::accumulateSym (
	Double& npts, AccumType& nvariance,
	AccumType& sumsq, const AccumType& datum, const AccumType& center
) {
	_NQUADSYM
}

template <class AccumType> void StatisticsUtilities<AccumType>::waccumulateSym (
	Double& npts, AccumType& sumweights, AccumType& wnvariance, AccumType& wsumsq,
	const AccumType& datum, const AccumType& weight, const AccumType& center
) {
	_WQUADSYM
}

template <class AccumType> template <class LocationType>
void StatisticsUtilities<AccumType>::accumulateSym (
	Double& npts, AccumType& nvariance,
	AccumType& sumsq, AccumType& datamin,
	AccumType& datamax, LocationType& minpos, LocationType& maxpos,
	const AccumType& datum, const LocationType& location, const AccumType& center
) {
	_NQUADSYM
	_MAXMINSYM
}

template <class AccumType> template <class LocationType>
void StatisticsUtilities<AccumType>::waccumulateSym (
	Double& npts, AccumType& sumweights, AccumType& wnvariance,
	AccumType& wsumsq, AccumType& datamin, AccumType& datamax,
	LocationType& minpos, LocationType& maxpos,
	const AccumType& datum, const AccumType& weight, const LocationType& location,
	const AccumType& center
) {
	_WQUADSYM
	_MAXMINSYM
}

template <class AccumType>
Bool StatisticsUtilities<AccumType>::includeDatum(
    const AccumType& datum, typename DataRanges::const_iterator beginRange,
    typename DataRanges::const_iterator endRange, Bool isInclude
) {
    typename DataRanges::const_iterator riter = beginRange;
    while (riter != endRange) {
        if (datum >= (*riter).first && datum <= (*riter).second) {
            return isInclude;
        }
        ++riter;
    }
    return ! isInclude;
}

template <class AccumType>
void StatisticsUtilities<AccumType>::convertToAbsDevMedArray(
    std::vector<AccumType>& myArray, AccumType median
) {
    typename std::vector<AccumType>::iterator iter = myArray.begin();
    typename std::vector<AccumType>::iterator end = myArray.end();
    // I bet there's a nifty way to do this in one line in C++11
    for (; iter!=end; ++iter) {
        *iter = abs(*iter - median);
    }
}

template <class AccumType>
std::map<uInt64, AccumType> StatisticsUtilities<AccumType>::indicesToValues(
    std::vector<AccumType>& myArray, const std::set<uInt64>& indices
) {
    uInt64 arySize = myArray.size();
    ThrowIf(
        *indices.rbegin() >= arySize,
        "Logic Error: Index " + String::toString(*indices.rbegin()) + " is too large. "
        "The sorted array has size " + String::toString(arySize)
    );
    std::map<uInt64, AccumType> indexToValuesMap;
    std::set<uInt64>::const_iterator initer = indices.begin();
    std::set<uInt64>::const_iterator inend = indices.end();
    Int64 lastIndex = 0;
    while(initer != inend) {
        GenSort<AccumType>::kthLargest(
            &myArray[lastIndex], arySize - lastIndex, *initer - lastIndex
        );
        lastIndex = *initer;
        ++initer;
    }
    std::set<uInt64>::const_iterator iter = indices.begin();
    std::set<uInt64>::const_iterator end = indices.end();
    while (iter != end) {
        indexToValuesMap[*iter] = myArray[*iter];
        ++iter;
    }
    return indexToValuesMap;
}

template <class AccumType>
void StatisticsUtilities<AccumType>::makeBins(
    BinDesc& bins, AccumType minData, AccumType maxData,
    uInt maxBins, Bool allowPad
) {
    bins.nBins = maxBins;
    bins.minLimit = minData;
    AccumType maxLimit = maxData;
    if (allowPad) {
        AccumType pad = (maxData - minData)/1e3;
        if (pad == (AccumType)0) {
            // try to handle Int like AccumTypes
            pad = AccumType(1);
        }
        bins.minLimit -= pad;
        maxLimit += pad;
    }
    bins.binWidth = (maxLimit - bins.minLimit)/(AccumType)bins.nBins;
}

template <class AccumType>
void StatisticsUtilities<AccumType>::mergeResults(
    std::vector<std::vector<uInt64> >& bins, std::vector<CountedPtr<AccumType> >& sameVal,
    std::vector<Bool>& allSame, const PtrHolder<std::vector<std::vector<uInt64> > >& tBins,
    const PtrHolder<std::vector<CountedPtr<AccumType> > >& tSameVal,
    const PtrHolder<std::vector<Bool> >& tAllSame, uInt nThreadsMax
) {
    // merge results from individual threads (tBins, tSameVal, tAllSame)
    // into single data structures (bins, sameVal, allSame)
    for (uInt tid=0; tid<nThreadsMax; ++tid) {
        std::vector<std::vector<uInt64> >::iterator iter;
        std::vector<std::vector<uInt64> >::iterator end = bins.end();
        typename std::vector<CountedPtr<AccumType> >::iterator siter;
        typename std::vector<CountedPtr<AccumType> >::iterator send = sameVal.end();
        std::vector<Bool>::iterator aiter;
        uInt idx8 = ClassicalStatisticsData::CACHE_PADDING*tid;
        std::vector<std::vector<uInt64> >::const_iterator titer = tBins[idx8].begin();
        for (iter=bins.begin(); iter!=end; ++iter, ++titer) {
            std::transform(
                iter->begin(), iter->end(), titer->begin(),
                iter->begin(), std::plus<Int64>()
            );
        }
        typename std::vector<CountedPtr<AccumType> >::const_iterator viter = tSameVal[idx8].begin();
        std::vector<Bool>::const_iterator witer = tAllSame[idx8].begin();
        for (
            siter=sameVal.begin(), aiter=allSame.begin(); siter!=send;
            ++siter, ++viter, ++aiter, ++witer
        ) {
            if (! *aiter) {
                // won't have the same values, do nothing
            }
            if (*witer && *aiter) {
                if (
                    viter->null()
                    || (! siter->null() && *(*siter) == *(*viter))
                ) {
                    // no unflagged values in this chunk or both
                    // have the all the same values, do nothing
                }
                else if (siter->null()) {
                    siter->reset(new AccumType(*(*viter)));
                }
                else {
                    // both are not null, and they do not have
                    // the same values
                    siter->reset();
                    *aiter = False;
                }
            }
            else {
                // *aiter = True, *witer = False, all values are not the same
                siter->reset();
                *aiter = False;
            }
        }
    }
}

template <class AccumType>
StatsData<AccumType> StatisticsUtilities<AccumType>::combine(
    const std::vector<StatsData<AccumType> >& stats
) {
    StatsData<AccumType> res = initializeStatsData<AccumType>();
    typename std::vector<StatsData<AccumType> >::const_iterator iter = stats.begin();
    typename std::vector<StatsData<AccumType> >::const_iterator end = stats.end();
    static const AccumType zero = 0;
    static const AccumType one = 1;
    for (; iter!=end; ++iter) {
        if (! iter->max.null()) {
            if (res.max.null() || *(iter->max) > *res.max) {
                res.max = iter->max;
                res.maxpos = iter->maxpos;
            }
        }
        if (! iter->min.null()) {
            if (res.min.null() || *(iter->min) < *res.min) {
                res.min = iter->min;
                res.minpos = iter->minpos;
            }
        }
        AccumType sumweights = iter->sumweights + res.sumweights;
        AccumType mean = sumweights == zero ? zero
            : (iter->sumweights*iter->mean + res.sumweights*res.mean)/sumweights;
        AccumType nvariance = zero;
        if (sumweights > zero) {
            AccumType diff1 = iter->mean - mean;
            AccumType diff2 = res.mean - mean;
            nvariance = iter->nvariance + res.nvariance
                + iter->sumweights*diff1*diff1
                + res.sumweights*diff2*diff2;
        }
        res.masked = iter->masked || res.masked;
        res.mean = mean;
        res.npts = iter->npts + res.npts;
        res.nvariance = nvariance;
        res.sum = iter->sum + res.sum;
        res.sumsq = iter->sumsq + res.sumsq;
        res.sumweights = sumweights;
        res.variance = res.sumweights > one
            ? nvariance/(res.sumweights - one) : 0;
        res.rms = sqrt(res.sumsq/res.sumweights);
        res.stddev = sqrt(res.variance);
        res.weighted = iter->weighted || res.weighted;
    }
    return res;
}

template <class AccumType>
template <class DataIterator, class MaskIterator, class WeightsIterator>
uInt StatisticsUtilities<AccumType>::nThreadsMax(
    const StatsDataProvider<CASA_STATP> *const dataProvider
) {
    uInt nthr = OMP::nMaxThreads();
    if (nthr > 1) {
        if (dataProvider) {
            uInt n = dataProvider->getNMaxThreads();
            if (n > 0) {
                return n;
            }
        }
    }
    return nthr;
}

template <class AccumType>
uInt StatisticsUtilities<AccumType>::threadIdx() {
#ifdef _OPENMP
    uInt tid = omp_get_thread_num();
#else
    uInt tid = 0;
#endif
    return tid * ClassicalStatisticsData::CACHE_PADDING;
}

}

#endif
