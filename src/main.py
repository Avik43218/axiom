import numpy as np
from scipy.stats import skew
from abc import ABC, abstractmethod

class StudentPerformanceAnalyzer(ABC):

    def __init__(self, studentId: int, testScores: list, maxScores: list):
        self.studentId = studentId
        self.testScores = testScores
        self.maxScores = maxScores
        
        normalizedTestScores = np.array(self._normalizeAllScores())
        self.normalizedTestScores = normalizedTestScores
        self.maximumScore = np.max(normalizedTestScores)
        self.minimumScore = np.min(normalizedTestScores)

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
    
    @abstractmethod
    def __repr__(self):
        return "Base class for the core engine"

class ConsistencyAnalyzer(StudentPerformanceAnalyzer):

    def __init__(self, studentId: int, testScores: list, maxScores: list):
        super().__init__(studentId, testScores, maxScores)
        self.name = "Consistency Analyzer Engine"
        self.ALPHA = 0.3
        self.BETA = 0.7
    
    def _calculateVariance(self) -> float:
        variance = np.var(self.normalizedTestScores, ddof=0)
        return variance
    
    def _calculateMeanAbsoluteDeviation(self) -> float:
        sampleDataMean = self._calculateMean()
        absoluteDeviations = np.absolute(self.normalizedTestScores - sampleDataMean)
        mad = np.mean(absoluteDeviations)

        return mad
    
    def analyzeStudentConsistency(self) -> float:

        variance = self._calculateVariance()
        standardDeviation = self._calculateStandardDeviation()
        consistency = 1 / (1 + self.ALPHA*variance + self.BETA*standardDeviation)

        return consistency
    
    def __repr__(self):
        return f"Description: {self.name}"

class StatisticsAnalyzer(StudentPerformanceAnalyzer):

    def __init__(self, studentId: int, testScores: list, maxScores: list):
        super().__init__(studentId, testScores, maxScores)
        self.name = "Statistics Analyzer Engine"
    
    def _checkSkewness(self) -> float:
        skewness = skew(self.normalizedTestScores)
        return skewness
    
    def _calculatePercentiles(self) -> list:
        q1 = np.percentile(self.normalizedTestScores, 25)
        median = np.percentile(self.normalizedTestScores, 50)
        q3 = np.percentile(self.normalizedTestScores, 75)

        return {"q1": q1, "median": median, "q3": q3}
    
    def __repr__(self):
        return f"Description: {self.name}"
    
class DistributionAnalyzer(StudentPerformanceAnalyzer):

    def __init__(self, studentId: int, testScores: list, maxScores: list):
        super().__init__(studentId, testScores, maxScores)
        self.name = "Distribution Analyzer Engine"
        self.logarithmBase = 2
    
    def _calculateShannonEntropy(self) -> float:
        sampleData = self.normalizedTestScores
        probabilities = sampleData[sampleData > 0]

        h = -np.sum(probabilities * np.log2(probabilities))

        return float(h)
    
    def _calculateRange(self) -> float:
        scoreRange = self.maximumScore - self.minimumScore
        return scoreRange
    
    def _calculateCoefficientOfVariation(self) -> float:
        cv = self._calculateStandardDeviation() / self._calculateMean()
        return cv
    
    def __repr__(self):
        return f"Description: {self.name}"
