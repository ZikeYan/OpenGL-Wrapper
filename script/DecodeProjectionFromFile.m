function P = DecodeProjectionFromFile(path, w_img, h_img, w_atlas, h_atlas)
    fid = fopen(path);
    c = textscan(fid, '%d %d %f %f');
    fclose(fid);
    i = double(c{1}); 
    j = double(c{2});
    u = c{3};
    v = c{4};
    
    P= DecodeProjection(i, j, w_img, h_img, ...
                        u, v, w_atlas, h_atlas);
end

