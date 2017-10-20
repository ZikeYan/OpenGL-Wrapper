from os import walk

all_files = next(walk('.'))[2]
txt_files = filter(lambda x: x.split('.')[-1] == 'txt', all_files)
sorted(txt_files, key=lambda x: int(x[5:11]))

fw = open('calib.traj', 'w')
fw.write(str(len(txt_files)) + '\n')

for txt_file in txt_files:
  print 'operating ' + txt_file
  fr = open(txt_file, 'r')
  R = fr.readline().strip('\n').split(' ')
  t = fr.readline().strip('\n').split(' ')
  fr.close()
  print(t)

  print(' '.join(R[0:2]) + ' ' + t[0] + '\n')
  fw.write(' '.join(R[0:3]) + ' ' + t[0] + '\n')
  fw.write(' '.join(R[3:6]) + ' ' + t[1] + '\n')
  fw.write(' '.join(R[6:9]) + ' ' + t[2] + '\n')
  fw.write('0 0 0 1\n')
fw.close()