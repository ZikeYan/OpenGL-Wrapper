function M = Vec2Mat(w, h, c, V)

M = zeros(h, w, c);
for i = 1 : c
    M(:,:,i) = reshape(V(:,i), h, w);
end

end

