# Ray Tracing: The Rest of Your Life

## 一个简单的蒙特卡罗程序

在计算机图形学中，随机算法（Randomized Algorithms）随处可见，常见的随机算法有两种：蒙特卡罗算法（Monte Carlo）和拉斯维加斯算法（Las Vegas）。

拉斯维加斯算法总是产生正确的结果，但是不能保证需要多长时间才能得到这个正确结果。拉斯维加斯算法的经典例子是快速排序算法，快速排序算法总是会完成一个完全排序的列表，但完成所需的时间是随机的。下面的代码展示了一个使用拉斯维加斯算法选择单位球中随机点的例子，这段代码最终能找到单位球中的一个随机点，但是却无法预估需要多长时间找到这个点，可能只需要一次迭代，也有可能需要两次、三次、四次，甚至更长时间：

```c++
inline vec3 random_in_unit_sphere()
{
    while (true)
    {
        auto p = vec3::random(-1,1);
        if (p.length_squared() < 1)
            return p;
    }
}
```

而蒙特卡罗算法可能会产生正确的结果（并且经常会出错），它会给出一个答案的统计估计值，并且运行时间越长，这个估计值就会越准确。这意味着在某个时间点，当答案达到需要的准确度后，就可以停止计算。这种简单程序产生噪声但不断改进答案的基本特性正是蒙特卡罗方法的核心，特别适用于不需要极高精度的图形应用。**对于计算机图形学中光线追踪等特别复杂的问题，并不会把完美精确放在首位，而是更看重在合理的时间内得到一个接近正确的结果**。

### 估算 $\pi$

![fig-3.01-circ-square.jpg](/Notes/Images/RayTracingTheRestOfYourLife/fig-3.01-circ-square.jpg)

蒙特卡罗算法的经典例子是估算 $\pi$ 。假设在一个正方形内部有一个相切的圆，在这个正方形内部选择随机的点，那些最终落在圆内的随机点的比例应该与圆的面积成比例（精确的比例应该就是圆的面积与正方形面积的比值）：

$$
\frac{\pi r^2}{(2r)^2}=\frac{\pi}{4}
$$

因为 $r$ 会相互抵消，那么这个比例应该就是 $\frac{\pi}{4}$ 。现在可以通过这个精确的比值来估算 $\pi$ 值：假设圆的半径 `r = 1` ，圆心在正方形原点，通过在正方形内随机选择 `N` 个点，计算并统计出在圆内的点的数量，根据在圆内点的数量与总的采样点的比值，得到精确的比值表达式 $\frac{\pi}{4}$ 的值，通过将比值乘以 `4` 也就得到了 $\pi$ 的估值。代码如下：

```c++
#include "rtweekend.h"

#include <iostream>
#include <iomanip>

int main()
{
    std::cout << std::fixed << std::setprecision(12);

    int inside_circle = 0;
    int N = 10000;

    for (int i = 0; i < N; i++)
    {
        auto x = random_double(-1, 1);
        auto y = random_double(-1, 1);
        if (x * x + y * y < 1)
            inside_circle++;
    }

    std::cout << "Estimate of Pi = " << (4.0 * inside_circle) / N << '\n';
}
```

基于初始随机种子，不同计算机上的 $\pi$ 值会有所不同。我这里得到的结果是： `Estimate of Pi = 3.121200000000` 。

### 收敛（Convergence）

将代码修改为一直运行，并且每 `10000` 次输出一次 $\pi$ 值。可以发现，输出的结果很快就接近了 $\pi$ ，并且在慢慢的逼近更精确的 $\pi$ 值。这个显示出了蒙特卡罗方法随着样本数量的增多，会越来越接近准确的结果。

```c++
#include "rtweekend.h"

#include <iostream>
#include <iomanip>

int main()
{
    std::cout << std::fixed << std::setprecision(12);

    int inside_circle = 0;
    int runs = 0;
    while (true)
    {
        runs++;
        auto x = random_double(-1, 1);
        auto y = random_double(-1, 1);
        if (x * x + y * y < 1)
            inside_circle++;

        if (runs % 10000 == 0)
            std::cout << "\rEstimate of Pi = " << (4.0 * inside_circle) / runs;
    }
}
```

### 分层采样（抖动）（Stratified Samples, or Jittering）

在上面显示收敛的过程中，可以看到刚开始很快就接近了 $\pi$ ，而到了后面，逼近的速度在减慢，这是**边际收益递减法则（the Law of Diminishing Returns）**的一个例子，每个样本的贡献都比上一个样本少，这是蒙特卡罗方法的一个缺点。

通过分层采样（一般称作抖动）的方法来替代随机采样可以减轻这种递减效应。如下图所示，将正方形分割成一个个的网格，并在每个网格中采样一个样本：

![fig-3.02-jitter](/Notes/Images/RayTracingTheRestOfYourLife/fig-3.02-jitter.jpg)

下面代码展示了随机采样和分层采样的对比，采样 `10000` 个样本：

```c++
#include "rtweekend.h"

#include <iostream>
#include <iomanip>

int main()
{
    std::cout << std::fixed << std::setprecision(12);

    int inside_circle = 0;
    int inside_circle_stratified = 0;
    int sqrt_N = 100;

    for (int i = 0; i < sqrt_N; i++)
    {
        for (int j = 0; j < sqrt_N; j++)
        {
            auto x = random_double(-1, 1);
            auto y = random_double(-1, 1);
            if (x * x + y * y < 1)
                inside_circle++;

            x = 2 * ((i + random_double()) / sqrt_N) - 1;
            y = 2 * ((j + random_double()) / sqrt_N) - 1;
            if (x * x + y * y < 1)
                inside_circle_stratified++;
        }
    }

    std::cout
        << "Regular    Estimate of Pi = "
        << (4.0 * inside_circle) / (sqrt_N * sqrt_N) << '\n'
        << "Stratified Estimate of Pi = "
        << (4.0 * inside_circle_stratified) / (sqrt_N * sqrt_N) << '\n';
}
```

输出的结果：

```zsh
Regular    Estimate of Pi = 3.150400000000
Stratified Estimate of Pi = 3.143200000000
```

可以看到，分层采样的方法比随机采样的方法要更好，而且以更好的渐近速率（Asymptotic Rate）收敛。但是这种优势会随着纬度的增加而减少。光线追踪就是一种非常高维的算法，每次反射都会增加两个新的维度： $\phi_o$ 和 $\theta_o$ 。

## 一维蒙特卡罗积分

### 期望值（Expected Value）

假设拥有以下条件：

1. 一个包含成员 $x_i$ 的值的列表 $X$ ：

    $$ X = (x_0, x_1, ... , x_{N-1}) $$

2. 一个连续的函数 $f(x)$ ，该函数接受列表中的成员作为输入：

    $$ y_i = f(x_i) $$

3. 一个函数 $F(X)$ ，它以列表 $X$ 作为输入，并输出列表 $Y$ ：

    $$ Y = F(X) $$

4. 其中输出列表 $Y$ 的成员 $y_i$ 为：

    $$ Y = (y_0, y_1, ... , y_{N-1}) = (f(x_0), f(x_1), ... , f(x_{N-1})) $$

那么，列表 $Y$ 的平均值可以表示为：

$$ \operatorname{average}(Y_i) = E[Y] = \frac{1}{N} \sum_{i=0}^{N-1} y_i $$

$$ = \frac{1}{N} \sum_{i=0}^{N-1} f(x_i) $$

$$ = E[F(X)] $$

其中， $E[Y]$ 被称为 $Y$ 的期望值。

平均值和期望值的区别：

- 通过不同随机采样的方法（不同的数量、不同的顺序等）从一个集合中随机采样，会生成出不同的子集合，每个子集合都有一个平均值，这个平均值就是所有采样的成员的总和除以总的采样数量。一个给定的成员在一个子集合中可能出现 0 次、1 次或者多次。
- **一个集合只有一个期望值：集合中所有成员的总和除以集合中的总项数**。也就是，期望值是集合中所有成员的平均值。
- 随着从一个集合中随机采样的数量增加，集合的平均值将会趋近于期望值。

如果 $x_i$ 是从连续区间 $[a, b]$ 中随机选择的，使得对所有 $i$ 而言 $a \leq x_i \leq b$ ，那么 $E[F(X)]$ 将近似于在相同区间 $a \leq x' \leq b$ 上，连续函数 $f(x')$ 的平均值。公式表达如下：

$$ E[f(x') | a \leq x' \leq b] \approx E[F(X)] | X = \{\small x_i | a \leq x_i \leq b \normalsize \} ] $$

$$ \approx E[Y = \{\small y_i = f(x_i) | a \leq x_i \leq b \normalsize \} ] $$

$$ \approx \frac{1}{N} \sum_{i=0}^{N-1} f(x_i) $$

如果取样本数量为 $N$ 并取极限（ $N \to \infty$ ），那么可以得到：

$$ E[f(x') | a \leq x' \leq b]  = \lim_{N \to \infty} \frac{1}{N} \sum_{i=0}^{N-1} f(x_i) $$

***在连续区间 $[a, b]$ 内，连续函数 $f(x')$ 的期望值可以通过在该区间内随机选取无限多的点并求和来完美表示。随着这些点的数量趋近于无穷大，输出的平均值越趋近于精确结果，这就是蒙特卡罗算法***。

在区间上求解期望值，随机采样点并不是唯一的方法，也可以选择放置采样点的位置。假设在区间 $[a, b]$ 上有 $N$ 个样本，均匀的分布这些点：

$$ x_i = a + i \Delta x $$

$$ \Delta x = \frac{b-a}{N} $$

求解此区间上的期望值：

$$ E[f(x') | a \leq x' \leq b] \approx \frac{1}{N} \sum_{i=0}^{N-1} f(x_i) \Big|_{x_i = a + i \Delta x} $$

$$ \approx \frac{\Delta x}{b - a} \sum_{i=0}^{N-1} f(x_i) \Big|_{x_i = a + i \Delta x} $$

$$ \approx \frac{1}{b - a} \sum_{i=0}^{N-1} f(x_i) \Delta x \Big|_{x_i = a + i \Delta x} $$

当 $N$ 趋近于无穷大时：

$$ E[f(x') | a \leq x' \leq b] = \lim_{N \to \infty} \frac{1}{b - a} \sum_{i=0}^{N-1} f(x_i) \Delta x \Big|_{x_i = a + i \Delta x} $$

此时，也就是解一个积分：

$$ E[f(x') | a \leq x' \leq b] = \frac{1}{b - a} \int_{a}^{b} f(x) dx $$

***函数的积分就是在该区间下曲线的面积***，所以：

$$ \operatorname{area}(f(x), a, b) = \int_{a}^{b} f(x) dx$$

因此，***一个区间上的平均值本质上与该区间下曲线的面积相关***：

$$  E[f(x) | a \leq x \leq b] = \frac{1}{b - a} \cdot \operatorname{area}(f(x), a, b) $$

函数的积分和蒙特卡罗采样都可以用于求解特定区间内的平均值。积分通过无限多的无穷小切片的总和来求解平均值，而蒙特卡罗算法通过在区间内逐渐增加的随机样本点的总和来近似相同的平均值。

### 积分 $x^2$

如下这个经典的积分：

$$ I = \int_{0}^{2} x^2 dx $$

使用积分法来求解这个积分：

$$ I = \frac{1}{3} x^3 \Big|_{0}^{2} $$

$$ I = \frac{1}{3} (2^3 - 0^3) $$

$$ I = \frac{8}{3} $$

或者使用蒙特卡罗方法来求解这个积分，表示为：

$$ I = \operatorname{area}( x^2, 0, 2 ) $$

可以写成：

$$  E[f(x) | a \leq x \leq b] = \frac{1}{b - a} \cdot \operatorname{area}(f(x), a, b) $$

$$ \operatorname{average}(x^2, 0, 2) = \frac{1}{2 - 0} \cdot \operatorname{area}( x^2, 0, 2 ) $$

$$ \operatorname{average}(x^2, 0, 2) = \frac{1}{2 - 0} \cdot I $$

$$ I = 2 \cdot \operatorname{average}(x^2, 0, 2) $$

代码实现如下：

```c++
#include "rtweekend.h"

#include <iostream>
#include <iomanip>

int main() {
    int a = 0;
    int b = 2;
    int N = 1000000;
    auto sum = 0.0;

    for (int i = 0; i < N; i++) {
        auto x = random_double(a, b);
        sum += x*x;
    }

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "I = " << (b - a) * (sum / N) << '\n';
}
```

输出：

```zsh
I = 2.666219812257
```

可以看到计算出的结果与积分得到的结果近似，即 $I = 2.666 \ldots = \frac{8}{3}$ 。在求解 $x^2$ 在 $[0, 2]$ 区间积分的情况下，求解实际上比求解蒙特卡罗方法工作量更少，但对于某些函数来说，求解蒙特卡罗方法可能比求解积分更简单。比如 $f(x) = \sin^5(x)$ ：

```c++
for (int i = 0; i < N; i++) {
    auto x = random_double(a, b);
    sum += std::pow(std::sin(x), 5.0);
}
```

还可以使用蒙特卡罗算法来处理无法进行解析积分的函数，例如 $f(x) = \ln(\sin(x))$ ：

```c++
for (int i = 0; i < N; i++) {
    auto x = random_double(a, b);
    sum += std::log(std::sin(x));
}
```

### 密度函数（Density Function）

<!-- 下图展示了一个直方图，它描述的是黑樱桃树的高度，其中 $x$ 轴表示的是高度的区间， $y$ 轴表示的是频率（数量）：

![fig-3.03-histogram](/Notes/Images/RayTracingTheRestOfYourLife/fig-3.03-histogram.jpg)

在直方图中，如果有更多的样本数量，那么区间（Bin）的数量将保持不变，但每个区间的频率会更高；如果将数据分成更多的区间，那么区间的数量会更多，但每个区间的频率会更低；如果将区间的数量提升到无穷大，那么将会产生无限个数量的 0 频率的区间。为了解决这个问题，可以使用 **离散密度函数（Discrete Density Function）** 来替换直方图（直方图是一个 **离散函数（Discrete Function）** ）。离散函数的直方图 $y$ 轴表示的是此区间的总计数，而离散密度函数的 $y$ 轴归一化为总量的一个比值（此区间的总计数/总量，也就是其密度）。从离散函数转换为离散密度函数的方法很简单，区间 $\text{Bin}_i$ 的密度可以通过此区间的总计数除以总的样本数来获得：

$$ \text{Density of Bin i} = \frac{\text{Number of items in Bin i}} {\text{Number of items total}} $$

一旦有了离散密度函数，就可以通过将离散值转换为连续值来将其转换为密度函数：

$$ \text{Bin Density} = \frac{(\text{Fraction of trees between height }H\text{ and }H’)} {(H-H’)} $$

因此，密度函数是一个连续的直方图，其中所有值都相对于总量进行了归一化。

如果有一颗特定的树想知道它的高度，可以通过创建一个 **概率函数（Probability Function）** 来表示其位于特定区间的可能性：

$$ \text{Probability of Bin i} = \frac{\text{Number of items in Bin i}} {\text{Number of items total}} $$

结合概率函数和连续的密度函数，就可以得到一个树高的统计预测函数：

$$ \text{Probability a random tree is between } H \text{ and } H’ = \text{Bin Density}\cdot(H-H’)$$

通过这个连续的概率函数，可以知道任意给定高度的树处于任意多个区间的可能性，这就是 **概率密度函数（Probability Density Function，PDF）** 。 PDF 是一个连续函数，可以对其进行积分以确定结果在一个积分范围内的可能性。 -->
