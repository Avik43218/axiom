"""
The core engine responsible for computing **individual student performance** based
on the marks obtained in previous examinations.

This engine consists of three sub-engines, which are child classes of an  
Abstract Base Class, with different roles:

--> |Consistency Compute Engine|: For computing the consistency score of the student
which is essential to know how much consistent the student is

--> |Statistics Compute Engine|: For computing basic statistical parameters, for
appropriate and unbiased analysis of the student at grassroot level

--> |Distribution Compute Engine|: For computing the marks distribution and checking
outliers, which is essential for estimating student potential


The inner workings of this engine involves two major steps, which are as follows:

1. Since marks records are often messy and inconsistent, the first step is to clean
them and normalize the scores. This is achieved by checking first whether marks obtained
are strictly lesser than or equal to maximum marks or not, or whether there is any 
NaN value or not, after which the marks are normalized on a scale of 10, using the
formula:

        (obtained_score / maximum_score) * 10.0

2. The next step involves, using the normalized scores to compute some of the most
widely used statistical parameters such as mean, median, and standard deviation
to name a few.

All the computed parameters are then returned as a dictionary, by a wrapper class
which puts all of them at a singular place.
"""


import numpy as np
from scipy.stats import skew
from abc import ABC, abstractmethod

class StudentPerformanceComputeEngine(ABC):

    def __init__(self, testScores: list, maxScores: list):
        self.name = "Basic Compute Engine"
        self.testScores = testScores
        self.maxScores = maxScores
        
        normalizedTestScores = np.array(self._normalizeAllScores())
        self.normalizedTestScores = normalizedTestScores
        self.maximumScore = np.max(normalizedTestScores)
        self.minimumScore = np.min(normalizedTestScores)

    @abstractmethod
    def _engineDescription(self) -> str:
        return "Base: Basic compute engine"

    @staticmethod
    def _normalizeScore(maxScore: int, obtainedScore: int) -> float:
        # Check for NaN, since NaN != NaN returns True
        if obtainedScore != obtainedScore:
            return 0.0

        if (maxScore != 0 and maxScore == maxScore) and obtainedScore <= maxScore:
            normalized = (obtainedScore / maxScore) * 10.0
        else:
            raise ValueError("Obtained score <= Maximum score (!= 0)")

        return normalized
    
    def _normalizeAllScores(self) -> list:
        normalizedTestScores = []
        for score, maxScore in zip(self.testScores, self.maxScores):
            normalizedScore = self._normalizeScore(maxScore, score)
            normalizedTestScores.append(normalizedScore)

        return normalizedTestScores
    
    def _calculateMean(self) -> float:
        mean = np.mean(self.normalizedTestScores)
        return mean
    
    def _calculateStandardDeviation(self) -> float:
        std = np.std(self.normalizedTestScores)
        return std
    
    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"

class ConsistencyComputeEngine(StudentPerformanceComputeEngine):

    def __init__(self, testScores: list, maxScores: list):
        super().__init__(testScores, maxScores)
        self.name = "Consistency Compute Engine"
        self.ALPHA = 0.3
        self.BETA = 0.7

    def _engineDescription(self):
        return "Sub: Consistency Compute Engine"
    
    def _calculateVariance(self) -> float:
        variance = np.var(self.normalizedTestScores, ddof=0)
        return variance
    
    def _calculateMeanAbsoluteDeviation(self) -> float:
        sampleDataMean = self._calculateMean()
        absoluteDeviations = np.absolute(self.normalizedTestScores - sampleDataMean)
        mad = np.mean(absoluteDeviations)

        return mad
    
    def returnConsistencyScores(self) -> dict:

        variance = self._calculateVariance()
        standardDeviation = self._calculateStandardDeviation()
        consistency = 1 / (1 + self.ALPHA*variance + self.BETA*standardDeviation)

        return {
            "consistency": consistency,
            "mad": self._calculateMeanAbsoluteDeviation()
            }
    
    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"

class StatisticsComputeEngine(StudentPerformanceComputeEngine):

    def __init__(self, testScores: list, maxScores: list):
        super().__init__(testScores, maxScores)
        self.name = "Statistics Compute Engine"

    def _engineDescription(self):
        return "Sub: Statistics Compute Engine"
    
    def _checkSkewness(self) -> float:
        skewness = skew(self.normalizedTestScores)
        return skewness
    
    def _calculatePercentiles(self) -> dict:
        q1 = np.percentile(self.normalizedTestScores, 25)
        median = np.percentile(self.normalizedTestScores, 50)
        q3 = np.percentile(self.normalizedTestScores, 75)

        return {"q1": q1, "median": median, "q3": q3}
    
    def returnStatistics(self) -> dict:
        coreStats = {
            "mean": self._calculateMean(),
            "skewness": self._checkSkewness(),
            "min": self.minimumScore,
            "max": self.maximumScore
        }
        percentileStats = self._calculatePercentiles()
        combinedStats = coreStats | percentileStats

        return combinedStats
    
    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"
    
class DistributionComputeEngine(StudentPerformanceComputeEngine):

    def __init__(self, testScores: list, maxScores: list):
        super().__init__(testScores, maxScores)
        self.name = "Distribution Compute Engine"
        self.logarithmBase = 2

    def _engineDescription(self):
        return "Sub: Distribution Compute Engine"
    
    def _calculateShannonEntropy(self) -> float:
        sampleData = self.normalizedTestScores
        filteredData = sampleData[sampleData > 0]

        probabilities = filteredData / np.sum(filteredData)

        h = -np.sum(probabilities * np.log2(probabilities))

        return float(h)
    
    def _calculateRange(self) -> float:
        scoreRange = self.maximumScore - self.minimumScore
        return scoreRange
    
    def _calculateCoefficientOfVariation(self) -> float:
        cv = self._calculateStandardDeviation() / self._calculateMean()
        return cv
    
    def returnDistributionScores(self) -> dict:
        return {
            "entropy": self._calculateShannonEntropy(),
            "range": self._calculateRange(),
            "cv": self._calculateCoefficientOfVariation(),
            "std": self._calculateStandardDeviation()
        }
    
    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"


class ComputeStudentPerformance:

    def __init__(self, studentId: int, testScores: list, maxScores: list):
        self.name = "Compute Engine Wrapper Class"

        self.studentId = studentId
        self.testScores = testScores
        self.maxScores = maxScores

        self.consistencyScoresObj = ConsistencyComputeEngine(testScores, maxScores)
        self.statisticsObj = StatisticsComputeEngine(testScores, maxScores)
        self.distributionScoresObj = DistributionComputeEngine(testScores, maxScores)

    def compute(self) -> dict:
        consistencyScores = self.consistencyScoresObj.returnConsistencyScores()
        statistics = self.statisticsObj.returnStatistics()
        distributionScores = self.distributionScoresObj.returnDistributionScores()

        allScores = consistencyScores | statistics | distributionScores

        return allScores
    
    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"
