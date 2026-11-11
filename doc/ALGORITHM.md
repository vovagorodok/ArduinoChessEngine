# Algorithm

The search is fundamentally based on **Negamax** with **Alpha-Beta Pruning**.

<img src="./images/negamax_with_alpha_beta.svg">

The search further incorporates **Null Move Pruning**, **Reverse Futility Pruning**, **Lazy Evaluation** and **Quiescence**.

<img src="./images/algorithm.svg">

Instead of searching to a fixed depth, the algorithm uses **Iterative Deepening** with **PV-Move** and **Aspiration Windows**.
