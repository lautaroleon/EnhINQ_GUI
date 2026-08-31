/*
This file is part of Time Tagger software defined digital data acquisition.

Copyright (C) 2011-2019 Swabian Instruments
All Rights Reserved

Unauthorized copying of this file is strictly prohibited.
*/

#pragma once

#include "TimeTagger.h"

/**
 * \defgroup ITERATOR All measurements and virtual channels
 *
 * \brief Base iterators for photon counting applications
 */

/**
 * \defgroup EventCounting Event counting
 * \ingroup ITERATOR
 */

/**
 * \defgroup TimeHistograms Time histograms
 * \ingroup ITERATOR
 *
 * \brief This section describes various measurements that calculate time differences between events and accumulate the
 * results into a histogram.
 */

/**
 * \defgroup FLIM Fluorescence-lifetime imaging (FLIM)
 * \ingroup ITERATOR
 *
 * \brief This section describes the Flim related measurements classes of the Time Tagger API.
 */

/**
 * \defgroup PhaseAndFrequency Phase & frequency analysis
 * \ingroup ITERATOR
 *
 * \brief This section describes measurement classes that expect periodic signals.
 */

/**
 * \defgroup TimeTagStreaming Time-tag-streaming
 * \ingroup ITERATOR
 *
 * \brief Measurement classes described in this section provide direct access to the time tag stream with minimal or no
 * pre-processing.
 */

/**
 * \defgroup HelperClasses Helper classes
 * \ingroup ITERATOR
 */

/**
 * \defgroup VirtualChannels Virtual Channels
 * \ingroup ITERATOR
 *
 * Virtual channels are software-defined channels as compared to the real input channels. Virtual channels can be
 * understood as a stream flow processing units. They have an input through which they receive time-tags from a real or
 * another virtual channel and output to which they send processed time-tags.
 *
 * Virtual channels are used as input channels to the measurement classes the same way as real channels. Since the
 * virtual channels are created during run-time, the corresponding channel number(s) are assigned dynamically and can be
 * retrieved using getChannel() or getChannels() methods of virtual channel object.
 */

/**
 * \brief Namespace for features, which are still in development and are likely to change.
 */
namespace Experimental {} // namespace Experimental

#include "measurements/ChannelGate.h"
#include "measurements/Coincidence.h"
#include "measurements/Combinations.h"
#include "measurements/Combiner.h"
#include "measurements/ConditionalFilterChannel.h"
#include "measurements/ConstantFractionDiscriminator.h"
#include "measurements/Correlation.h"
#include "measurements/CorrelationPairs.h"
#include "measurements/Counter.h"
#include "measurements/Countrate.h"
#include "measurements/CustomMeasurementBase.h"
#include "measurements/DelayedChannel.h"
#include "measurements/Dump.h"
#include "measurements/EventGenerator.h"
#include "measurements/FileReader.h"
#include "measurements/FileWriter.h"
#include "measurements/Flim.h"
#include "measurements/FrequencyCounter.h"
#include "measurements/FrequencyMultiplier.h"
#include "measurements/FrequencyStability.h"
#include "measurements/GatedChannel.h"
#include "measurements/GatedCounter.h"
#include "measurements/HistogramLogBins.h"
#include "measurements/HistogramND.h"
#include "measurements/Iterator.h"
#include "measurements/MarkovProcessGenerator.h"
#include "measurements/OverflowInjector.h"
#include "measurements/PhaseNoise.h"
#include "measurements/PhotonNumber.h"
#include "measurements/PulsePerSecondMonitor.h"
#include "measurements/Sampler.h"
#include "measurements/Scope.h"
#include "measurements/SignalGenerators.h"
#include "measurements/Simulation.h"
#include "measurements/StartStop.h"
#include "measurements/SynchronizedMeasurements.h"
#include "measurements/SyntheticSingleTag.h"
#include "measurements/TimeDifferences.h"
#include "measurements/TimeDifferencesND.h"
#include "measurements/TimeTagStream.h"
#include "measurements/TriggerOnCountrate.h"
