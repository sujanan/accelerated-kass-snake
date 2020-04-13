import math
import numpy as np
import PyDIP as dip
import matplotlib.pyplot as plt

def show_img(img):
    plt.imshow(img, cmap='gray')
    plt.show()


def gen_init_contour():
    contx = np.arange(0, 2 * math.pi, 0.1)
    conty = np.arange(0, 2 * math.pi, 0.1)
    xlamda = lambda x: 120 + 50 * math.cos(x)
    ylamda = lambda y: 140 + 60 * math.sin(y)
    xfunc = np.vectorize(xlamda)
    yfunc = np.vectorize(ylamda)
    return (xfunc(contx), yfunc(conty))


def plot_contour(img, contx, conty, grayval):
    for i in range(len(contx)):
        img[int(contx[i]), int(conty[i])] = grayval


class Snake():
    def __init__(self, img, contx, conty, exteng, alpha, beta, gamma, niter=50):
        self.img = img
        self.contx = contx
        self.conty = conty
        self.exteng = exteng
        self.alpha = alpha
        self.beta = beta
        self.gamma = gamma
        self.niter = niter
        self.mat = self._create_matrix()

    def _create_matrix(self):
        n = len(self.contx)
        a = self.gamma * (2 * self.alpha + 6 * self.beta) + 1
        b = self.gamma * (-self.alpha - 4 * self.beta)
        c = self.gamma * self.beta
        mat = np.empty(shape=(n, n))
        for i in range(n):
            in2 = n + (i - 2) if (i - 2) < 0 else (i - 2)
            in1 = n + (i - 1) if (i - 1) < 0 else (i - 1)
            ip1 = (i + 1) % n
            ip2 = (i + 2) % n
            for j in range(n):
                if in2 == j:
                    mat[i][j] = c
                elif in1 == j:
                    mat[i][j] = b
                elif i == j:
                    mat[i][j] = a
                elif ip1 == j:
                    mat[i][j] = b
                elif ip2 == j:
                    mat[i][j] = c
                else:
                    mat[i][j] = 0.0
        mat = np.linalg.inv(mat)
        return mat

    def exec(self, plot_intervel, plot_color=255.0):
        n = len(self.contx)
        for i in range(self.niter):
            fex = np.empty(n)
            fey = np.empty(n)
            for j in range(len(self.contx)):
                coords = [int(round(self.contx[j])), int(round(self.conty[j]))]
                fex[j] = dip.SubpixelLocation(self.exteng.TensorRow(0), coords, method='linear').value
                fey[j] = dip.SubpixelLocation(self.exteng.TensorRow(1), coords, method='linear').value
            self.contx = self.mat.dot(self.contx + (self.gamma * fex))
            self.conty = self.mat.dot(self.conty + (self.gamma * fey))
            if plot_intervel and i % plot_intervel == 0:
                plot_contour(self.img, self.contx, self.conty, plot_color)
        if plot_intervel:
            plot_contour(self.img, self.contx, self.conty, plot_color)


if __name__ == '__main__':
    img = dip.ImageReadICS('../res/img')
    exteng = dip.Gradient(dip.GradientMagnitude(img, [30]))
    contx, conty = gen_init_contour()
    snake = Snake(img, contx, conty, exteng, alpha=0.001, beta=0.4, gamma=100, niter=50)
    snake.exec(5)
    show_img(img)
