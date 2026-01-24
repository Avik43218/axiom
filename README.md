# Performance Analysis Engine (perf-engine)

## Introduction

A modular, mathematically grounded system designed to analyze, normalize and transform raw student marks into comparable and workable performance metrics, and visualize the performance metrics for additional clarity and representation.

The project is built as a **multi-engine analytical pipeline**, where each engine focuses on a specific level of insight: from idividual students to aggregate class behaviour to visualization.

## Architecture Breakdown

### 1. Student Performance Compute Engine

The core engine responsible for computing student performance trends based on the record of marks obtained in previous exams.

This engine can be further broken down into **three sub-engines**:
- **Consistency Compute Engine:** The sub-engine responsible for computing consistency trends of the student, using different parameters and techniques.
- **Statistics Compute Engine:** This sub-engine is responsible for computing basic statistical parameters, such as mean, percentiles, etc.
- **Distribution Compute Engine:** Marks distribution and outliers is computed by this sub-engine, with the help of various standard statistical methods. 

## Tech Stack

- **Programming Languages:** Python3
- **Frameworks/Libraries:** NumPy, SciPy
- **Tools:** Git

## Future Enhancements 

These are the features that are either under development or are being planned to be included in the near future.

Some of them include:

- Class aggregate score compute engine
- Final score validation, comparison and display engine
- A modern web page for viewing the data efficiently

## License

This project is licensed and maintained under the **GNU General Public License 3.0**.

