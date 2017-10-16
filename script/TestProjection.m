Ps = cell(89, 1);


for viewpoint = 1 : 89    
    fid = fopen(sprintf('bin/map.txt'));
    c = textscan(fid, '%d %d %f %f');
    fclose(fid);
    i = double(c{1}); 
    j = double(c{2});
    u = c{3};
    v = c{4};

    atlas = im2double(imread('model/beethoven/beethoven.png'));
    %atlas = im2double(imread('bin/shaded-texture.png')) ...
    %     .* im2double(imread('obj/face.png'));

    tic;
    h_atlas = size(atlas, 1);
    w_atlas = size(atlas, 2);
    h_img = 480;
    w_img = 640;

    Ps{viewpoint} = DecodeProjection(i, j, w_img, h_img, ...
                                     u, v, w_atlas, h_atlas);
    v_gen = Ps{viewpoint} * Mat2Vec(w_atlas, h_atlas, 3, atlas);

    toc;

    %im_ren = imread('bin/im_render.png');
    im_matlab = im2uint8(Vec2Mat(w_img, h_img, 3, v_gen));
    %im_gl     = imread(sprintf('bin/im_gl%d.png', viewpoint-1));
    %im_real   = imread(sprintf('../../data/Face/Frame000/Image%d.png', viewpoint));

    figure(1);
    subplot(1, 3, 1);
    imshow(im_matlab);
    title('matlab');

%     subplot(2, 3, 2);
%     imshow(im_gl);
%     title('gl');

    %subplot(1, 3, 2);
    %imshow(im_real);
    %title('real');

    %subplot(1, 3, 3);
    %imshow(10*abs(im_matlab - im_gl));
    %title('diff matlab gl');
% 
%     subplot(2, 3, 5);
%     imshow(abs(im_matlab - im_real));
%     title('diff matlab real');
% 
%     subplot(2, 3, 6);
%     imshow(abs(im_gl - im_real));
%     title('diff gl real');

    drawnow;
end