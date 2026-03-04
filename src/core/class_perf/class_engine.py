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
            "consistency", "mad", "mean", "skewness",
            "min", "max", "q1", "median", "q3",
            "entropy", "cv", "range", "std"
        ]

        metrics = {name: [] for name in metricNames}

        for value in self.allStudentsScoresDict.values():
            for name in metricNames:
                metrics[name].append(value[name])

        return {name: np.array(values) for name, values in metrics.items()}

