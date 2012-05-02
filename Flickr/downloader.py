import flickr
import urllib, urlparse
import os
import sys
import errno
import os.path

tag = ''
num_per_page = 10
num_pages = 1
starting_page = 1
#Parsing user input
if len(sys.argv) == 3:
    tag =sys.argv[1]
    num_per_page = sys.argv[2]
elif len(sys.argv) == 2:
    tag = sys.argv[1]
elif len(sys.argv) == 4:
		tag = sys.argv[1]
		num_per_page = sys.argv[2]
		num_pages = int(sys.argv[3])
elif len(sys.argv) == 5:
		tag = sys.argv[1]
		num_per_page = sys.argv[2]
		num_pages = int(sys.argv[3])
		starting_page = int(sys.argv[4])
else:
    print 'Usage: python2.7 <tags .... > num_per_page'
    exit()

#Print tags
print 'Tags: ', tag
print 'Per page: ', num_per_page
print 'Num pages: ', num_pages
print 'Starting page: ', starting_page
new_directory = 'images/'+tag

try:
	os.makedirs(new_directory)
except OSError as exc:
	if exc.errno == errno.EEXIST:
		pass
	else: raise

fl = open(new_directory+'/urllist.txt','w')

img_cnt = starting_page*num_per_page

#For each page
for i in range(starting_page, starting_page+num_pages+1):
	print 'Page: ', i
  
	#Get all photos
	fotos = flickr.photos_search(tags=tag, per_page=num_per_page, page=i)
	urllist = []
  
	#Get all photos out of request -> Now just gets urls
	for photo in fotos:
		
		#Get url
		url = photo.getURL(size='Small', urlType='source')
                print_str = str(img_cnt) + ' Downloading: ' + url
		print print_str
		urllist.append(url)
		
		#Write url to url list
		fl.write(new_directory+'/'+url+'\n')
		
		#Download image
		web_file = urllib.urlopen(url)
                write_url = os.path.basename(url)
                print 'Downloading: ', write_url
		local_file = open(new_directory+'/'+write_url, 'w')
		local_file.write(web_file.read())
		web_file.close()
		local_file.close()

		#Increment Image count
		img_cnt = int(img_cnt) + 1
fl.close()
