from abc import ABC, abstractmethod

class AxiomWrapperClass(ABC):

    def __init__(self):
        self.name = "Axiom Wrapper Class"

    @abstractmethod
    def description(self) -> str:
        return "Description: Axiom Wrapper Class for the compute engines"

