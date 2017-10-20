% To test this, first compile and run the C++ code
% ./encode_image_pixel2uv_projection
atlas = im2double(imread('../../model/beethoven/beethoven.png'));

tic;
h_atlas = size(atlas, 1);
w_atlas = size(atlas, 2);
h_img = 960;
w_img = 1280;

P = DecodeProjectionFromFile('../../bin/beethoven_projection_map.txt', ...
    w_img, h_img, w_atlas, h_atlas);
v_gen = P * Mat2Vec(w_atlas, h_atlas, 3, atlas);
toc;

im_matlab = im2uint8(Vec2Mat(w_img, h_img, 3, v_gen));
im_gl = imread('../../bin/beethoven_screenshot.png');

figure(1);
subplot(1, 2, 1);
imshow(im_matlab);
title('matlab');

subplot(1, 2, 2);
imshow(im_gl);
title('gl');

%     subplot(1, 3, 3);
%     imshow(im_real);
%     title('real');

drawnow;