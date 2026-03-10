# Axiom - Performance Metrics Computation System

## Introduction

A modular, mathematically grounded system designed to analyze, normalize and transform raw student marks into comparable and workable performance metrics, and visualize the performance metrics for additional clarity and representation.

The project is built as a **multi-engine analytical pipeline**, where each engine focuses on a specific level of insight: from individual students to aggregate class behaviour to visualization.

## Architecture Breakdown

### 1. Data Preprocessor Engine

This is the core engine responsible for cleaning & normalization of the raw & messy data, and inserting them into an SQL database.

It takes in raw CSV data files, extracts the student ID along with their test scores, normalizes the scores, and inserts both the raw scores and normalized scores into separate tables for proper comparison and evaluation of the students.

The formula used for normalization is:

        N = (X / M) * S
        
        where, 
        N = Normalized score
        X = Score obtained by student
        M = Maximum score
        S = Scaling factor

_Note:_ The ratio of X and M is multiplied by a large integer scaling factor to convert floating point precision to inetger precision and avoid rounding-off errors in different systems.
        

### 2. Student Performance Compute Engine

The core engine responsible for computing student performance trends based on the record of marks obtained in previous exams.

This engine can be further broken down into **three sub-engines**:
- **Consistency Compute Engine:** The sub-engine responsible for computing consistency trends of the student, using different parameters and techniques.
- **Statistics Compute Engine:** This sub-engine is responsible for computing basic statistical parameters, such as mean, percentiles, etc.
- **Distribution Compute Engine:** Marks distribution and outliers is computed by this sub-engine, with the help of various standard statistical methods. 

### 3. Class Performance Compute Engine

This core engine is responsible for computing the ranks and percentiles of all students in a class, and also oraganize the individual performance metrics into a neat data structure.

This engine is comprised of **two sub-engines**:
- **Metrics Organizer Engine:** It is responsible for organizing the individual students' metrics into groups for ease of access and future evaluation.
- **Rankings Compute Engine:** This sub-engine is responsible for computing the ranks and percentiles of the students in a class, along with the Z-Scores of standard deviations to measure the volatility of their consistency. 

## Tech Stack

- **Programming Languages:** Python3, C++, Bash
- **Frameworks/Libraries:** NumPy, SciPy
- **Tools:** Git

## Future Enhancements 

These are the features that are either under development or are being planned to be included in the near future.

Some of them include:

- A system for checking data & file integrity
- Final score validation, comparison and display engine
- A modern web page for viewing the data efficiently

---

## License

This project is licensed and maintained under the **GNU General Public License 3.0**.

