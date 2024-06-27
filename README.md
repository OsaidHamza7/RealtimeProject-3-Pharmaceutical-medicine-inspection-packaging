# Pharmaceutical Medicine Inspection & Packaging Simulation

This project is a multi-processing and multi-threading application that simulates the inspection and packaging process of pharmaceutical medicines. It models the workflows in a pharmaceutical factory focusing on the manual inspection and packaging stages of production for liquid and pill-based medicines.

## Demo
[Demo.webm](https://github.com/OsaidHamza7/RealtimeProject-3-Pharmaceutical-medicine-inspection-packaging/assets/108472343/00512847-81c8-4175-9fd9-1e58a454826c)


## Project Description

The simulation is designed to represent a pharmaceutical factory that produces a user-defined number of different medicines on various production lines. It aims to mirror real-life scenarios where medicines are inspected and packaged by employees to ensure quality and compliance before distribution.

### Features

- **Simulation of multiple production lines:** Some lines produce liquid medicine while others produce pill-based medicine.
- **Detailed inspection criteria:**
  - **Liquid medicine checks:** Liquid level, color, sealing, labeling, and expiry dates.
  - **Pill-based medicine checks:** Pill count, color, size, and expiry dates on containers.
- **Dynamic employee allocation:** Employees can be shifted between production lines to optimize the inspection and packaging process based on the production speed and demands.
- **User-defined thresholds and conditions** for production amounts, quality specs, and simulation duration.


## Installation

1. Clone the repository:
   ````bash
   git clone https://github.com/OsaidHamza7/RealtimeProject-3-Pharmaceutical-medicine-inspection-packaging.git
   ````
2. Navigate to the project directory:
   ```bash
   cd RealtimeProject-3-Pharmaceutical-medicine-inspe tion-packaging
   ```
3. Build the project:
   ```bash
   make all
   ```

## Usage

1. Run the simulation:
   ```bash
   make run
   ```
2. Follow the prompts to configure the simulation settings such as the number of production lines, production thresholds, and quality specifications.

3. Monitor the output which includes detailed logs of the inspection and packaging processes, employee activities, and end-of-simulation summaries.

## Authors

- **Osaid Hamza - Team Leader**
- Razan Abdelrahman
- Misam Alaa
- Ansam Rihan
