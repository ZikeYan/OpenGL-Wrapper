function V = Mat2Vec(w, h, c, M)

V = zeros(w*h, c);
for i = 1 : c
    V(:,i) = reshape(M(:,:,i), w*h, 1);
end

end

