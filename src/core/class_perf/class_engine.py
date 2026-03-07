from abc import ABC, abstractmethod
import numpy as np
import json

class ClassPerformanceComputeEngine(ABC):

    def __init__(self, allStudentsScoresJSON: str):
        self.name = "Class Aggregate Performance Compute Engine"
        self.allStudentsScoresJSON = allStudentsScoresJSON

    @abstractmethod
    def _engineDescription() -> str:
        return "Base: Class Aggregate Performance Compute Engine base class"

    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"


class MetricsOrganizerEngine(ClassPerformanceComputeEngine):
    
    def __init__(self, allStudentsScoresJSON: str):
        super().__init__(allStudentsScoresJSON)
        self.name = "Metrics Organization Engine"

        self.allStudentsScoresDict = dict(json.loads(allStudentsScoresJSON))
        self.numberOfStudents = len(self.allStudentsScoresDict)

    def _engineDEscription() -> str:
        return "Sub: Sub-class for organising students performance metrics"

    def organizeMetrics(self) -> dict:

        metricNames = [
            "studentId", "total", "std", "mean", "min",
            "max", "consistency", "skewness", "entropy",
            "q1", "median", "q3", "mad", "range", "cv"
        ]

        metrics = {name: [] for name in metricNames}

        for value in self.allStudentsScoresDict.values():
            for name in metricNames:
                metrics[name].append(value[name])

        return {name: np.array(values) for name, values in metrics.items()}

    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"


class StudentRankingComputeEngine(ClassPerformanceComputeEngine):

    def __init__(self, allStudentsScoresJSON: str, groupMetrics: dict):
        super().__init__(allStudentsScoresJSON)
        self.name = "Student Ranking Compute Engine"
        self.groupedMetrics = groupMetrics

    def _engineDescription(self) -> str:
        return "Sub: Sub-engine for computing percentile and ranking of students in a class"

    def _computeZScoresStd(self) -> list:

        stdScores = self.groupMetrics["std"]

        M = np.mean(stdScores)
        S = np.std(stdScores)

        ZScoresStd = (stdScores - M) / S

        return list(ZScoresStd)

    def _computeRankings(self) -> list:

        stdScores = self.groupMetrics["std"]
        totalScores = self.groupMetrics["total"]

        indices = np.lexsort((stdScores, -totalScores))

        ranks = np.empty_like(indices)
        ranks[indices] = np.arange(1, len(indices) + 1)

        return list(ranks)

    def _computeClassPercentiles(self, ranks) -> list:
        
        N = len(ranks)
        ranks = np.array(ranks)

        percentiles = ((N - ranks + 1) / N) * 100

        return list(percentiles)

    def returnRankingMetrics(self) -> dict:
        return {
            "zScoresStd": self._computeZScoresStd(),
            "ranks": self._computeRankings(),
            "percentiles": self._computeClassPercentiles()
        }

    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"

    
class ComputeRankingsAndPercentiles:

    def __init__(self, allStudentsScoresJSON: str):
        self.name = "Class Performance Compute Engine Wrapper Class"
        self.allStudentsScoresJSON = allStudentsScoresJSON

        self.MetricsOrganizerObj = MetricsOrganizerEngine(self.allStudentsScoresJSON)
        self.groupMetrics = self.MetricsOrganizerObj.organizeMetrics()

        self.RankingAndPercentileComputeObj = StudentRankingComputeEngine(self.allStudentsScoresJSON, self.groupMetrics)

    def compute(self) -> str:

        rankingMetrics = self.RankingAndPercentileComputeObj.returnRankingMetrics()

        allMetrics = self.groupMetrics | rankingMetrics
        return json.dumps(allMetrics)

    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"

