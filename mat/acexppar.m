% load diplib
dip_initialise

% load image
img = noise(50 + 100*gaussf(rr>85, 2), 'gaussian', 20)
hold on

alpha = 0.001;
beta = 0.4;
gamma = 100;
niters = 40;

f = gradient(gradmag(img, 30));

% pick points from mouse
[px, py] = getpts;

for i = 1:length(px)-1
    if i == length(px)-1
        j = 1;
    else
        j = i + 1;
    end
    d1 = [px(i), py(i)];
    d2 = [px(j), py(j)];
    x0 = (d1(1) + d2(1))/2;
    y0 = (d1(2) + d2(2))/2;
    r = norm(d1-d2)/2;
    [x, y] = circlecon(x0, y0, r);
    plot(x, y, 'g')
    snake(f, x, y, alpha, beta, gamma, niters)
end

function [x, y] = circlecon(x0, y0, r)
    ang = 0:0.01:2*pi;
    x = x0 + r*cos(ang)';
    y = y0 + r*sin(ang)';
end

function snake(f, x, y, alpha, beta, gamma, niters)
    N = length(x);
    a = gamma*(2*alpha + 6*beta) + 1;
    b = gamma*(-alpha - 4*beta);
    c = gamma*beta;
    P = diag(repmat(a, 1, N));
    P = P + diag(repmat(b, 1, N-1), 1) + diag(    b, -N+1);
    P = P + diag(repmat(b, 1, N-1),-1) + diag(    b,  N-1);
    P = P + diag(repmat(c, 1, N-2), 2) + diag([c, c],-N+2);
    P = P + diag(repmat(c, 1, N-2),-2) + diag([c, c], N-2);
    P = inv(P);

    for ii = 1:niters
       % Calculate external force
       coords = [x, y];
       fex = get_subpixel(f{1}, coords, 'linear');
       fey = get_subpixel(f{2}, coords, 'linear');
       % Move control points
       x = P*(x + gamma*fex);
       y = P*(y + gamma*fey);
       if mod(ii, 5)==0
           plot([x;x(1)], [y;y(1)], 'b')
       end
    end
    plot(x, y, 'r')
end
