#EL 6483 RTES Project

##Index
---

- [##Index](#index)
- [Goal](#goal)
- [Hardware](#hardware)
- [Algorithm](#algorithm)
  - [Dynamic Time Warping:](#dynamic-time-warping)
  - [Cost function:](#cost-function)
  - [Extra constraint:](#extra-constraint)
- [Recognition procedure](#recognition-procedure)
- [Result](#result)

---

## Goal
Use the data collected from a single accelerometer to record a hand movement sequence as a means to generally “unlock” a resource.

`If you want to reproduce the recognition, please open serial monitor and follow reminder on it.`

---

## Hardware
![](e:/Git_repos/GESTURE-RECOGNITION-EMBEDDED-PROJECT/undefined/images/2020-06-20-22-37-01.png)

*Figure 1: Hardware*

(a) Board: SAMD21 XPLAINED PRO

(b) Sensor: MPU6050

(c) Extra: jumper wire, bread board

*To Use SAMD21 XPLAINED PRO with Arduino, refer to [Github][1].*

[1]: https://github.com/AtmelUniversityFrance/atmel-samd21-xpro-boardmanagermodule/wiki/GettingStarted:-SAMD21-Xplained-Pro

---

## Algorithm

### Dynamic Time Warping:
For the project, I used Dynamic Time Warping algorithm. In general, DTW is a method that calculates an optimal match between two given sequences (e.g. time series), shown in figure below.

![](e:/Git_repos/GESTURE-RECOGNITION-EMBEDDED-PROJECT/undefined/images/2020-06-20-22-37-58.png)

*Figure 2: Dynamic Time Warping*

---

### Cost function:

After calculating optimal match between two acceleration sequences, I used difference(L2-Norm) between sequences as a measurement for cost function, thus it can be use to recognize gesture by comparing threshold obtained by training and cost function value from each recognition.

After finding optimal match(detailed implementation won’t be shown here  , the cost function is computed by:

<!-- \[
Cost = \frac{\sum_{}^{}d[i][j] }{N}
\]
\[
d[i][j] = (1-scale * \frac{Acc_i*Acc_j}{\|Acc_i\|*\|Acc_j\| + 1*10^{-6}} ) * Norm
\] -->

![](e:/Git_repos/GESTURE-RECOGNITION-EMBEDDED-PROJECT/undefined/images/2020-06-20-22-51-59.png)

Cost is a mean value of d[i][j]. d[i][j] is the difference(norm) between two acceleration vectors of optimal match times a designed regularized compound coefficient. 

The coefficient enables a feature that if directions of two acceleration are more similar, the coefficient is smaller, if directions are more opposite, coefficient is larger, this adjust the intensity of alleviation/punishment. 

The reason why I want this feature is because direction is a more important factor need to be considered than pure value difference between optimal match, optimal match doesn’t mean two sequences are similar.

---

### Extra constraint:

More over, set a constraint for DTW back tracking for matching points
pairs, because I sample 50 points, so I limit back track steps not beyond 62.

If the constraint is violated, there is a possibility of compulsory matching for two much different sequences.

---

## Recognition procedure

(1) Train: press button to record template sequence and a comparison sequence of right gesture, use DTW and cost function to obtained a threshold value.

(2) Test: press button to record gesture, program will perform DTW and
cost function on this gesture sequence and the template sequence at the
beginning, obtain a cost value, then compare this cost with threshold with suitable tolerance.

(3) Result: if gesture is recognized as matched, LED will blink three times, also procedure and result can be viewed at serial monitor.

---

## Result

(1) tested with linear, circle, relative simple random gesture

(2) achieve a good success rate of random gesture that is not very complex

(3) due to feature of DTW and designed cost function, it can distinguish same gesture of opposite direction

(4) Only one ”run” bottom and a reset bottom are needed.

(5) Video link:
https://drive.google.com/file/d/1P_2Dh37riOLzZ1SX35Tt7MYHmsmLDjz6/viewusp=sharing
