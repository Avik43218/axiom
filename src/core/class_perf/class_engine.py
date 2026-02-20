import pandas as pd
import numpy as np

class ClassAggregateComputeEngine:

    def __init__(self):
        self.name = "Class Aggregate Performance Compute Engine"

    def __repr__(self):
        return f"{self.__class__.__name__}: {self.name}"