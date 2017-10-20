% To test this, first compile and run the C++ code
% ./encode_sequence_pixel2uv_projection
Ps = cell(89, 1);
for viewpoint = 1 : 89
    atlas = im2double(imread('../../model/face/face.png'));

    tic;
    h_atlas = size(atlas, 1);
    w_atlas = size(atlas, 2);
    h_img = 1920;
    w_img = 2560;

    Ps{viewpoint} = DecodeProjectionFromFile(...
        sprintf('../../bin/face/map_%d.txt', viewpoint-1), ...
        w_img, h_img, w_atlas, h_atlas);
    v_gen = Ps{viewpoint} * Mat2Vec(w_atlas, h_atlas, 3, atlas);
    toc;
    
    im_matlab = im2uint8(Vec2Mat(w_img, h_img, 3, v_gen));
    im_gl     = imread(sprintf('../../bin/face/image_%d.png', viewpoint-1));
    im_real   = imread(sprintf('../../../../../data/Face/Frame000/Image%d.png', viewpoint));

    figure(1);
    subplot(1, 3, 1);
    imshow(im_matlab);
    title('matlab');

    subplot(1, 3, 2);
    imshow(im_gl);
    title('gl');

    subplot(1, 3, 3);
    imshow(im_real);
    title('real');

    drawnow;
end