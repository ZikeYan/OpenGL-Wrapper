function P = DecodeProjection(i, j, w_img, h_img, u, v, w_atlas, h_atlas)

i = i + 1;
j = j + 1;
Si = repmat((j-1)*h_img+i, 9, 1);

v = (1-v)*h_atlas + 1;
u = u*w_atlas + 1;

v1 = min(round(v), h_atlas);
v0 = min(max(v1 - 1, 1), h_atlas);
v2 = min(v1 + 1, h_atlas);
u1 = min(round(u), w_atlas);
u0 = min(max(u1 - 1, 1), w_atlas);
u2 = min(u1 + 1, w_atlas);

g = @(x,y,x0,y0) (exp(-( (x-x0).*(x-x0) + (y-y0).*(y-y0) )/2 ));
g0 = g(u,v,u0,v0); g1 = g(u,v,u0,v1); g2 = g(u,v,u0,v2);
g3 = g(u,v,u1,v0); g4 = g(u,v,u1,v1); g5 = g(u,v,u1,v2);
g6 = g(u,v,u2,v0); g7 = g(u,v,u2,v1); g8 = g(u,v,u2,v2);
s = g0+g1+g2+g3+g4+g5+g6+g7+g8;

Sj = [(u0-1)*h_atlas+v0; (u0-1)*h_atlas+v1; (u0-1)*h_atlas+v2; ...
      (u1-1)*h_atlas+v0; (u1-1)*h_atlas+v1; (u1-1)*h_atlas+v2; ...
      (u2-1)*h_atlas+v0; (u2-1)*h_atlas+v1; (u2-1)*h_atlas+v2];

Sv = [g0; g1; g2; g3; g4; g5; g6; g7; g8];
Sv = Sv ./repmat(s, 9, 1);

P = sparse(Si, Sj, Sv, w_img*h_img, w_atlas*h_atlas);

end

