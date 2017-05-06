import matplotlib.pyplot as plt


rtt=[0.618178,0.605093,0.605751,0.604008,0.607982]
p=[0.152735,0.399235,1.114235,1.538235,2.332235]
bx=plt.subplot(111)
bx.plot(p,rtt,linestyle='--',marker='o',linewidth=1.5)
bx.grid()
bx.set_xlim(0,2.5)
bx.set_ylim(0.6,0.62)
bx.set_title('OPTIM algorithm',color='#000000',weight="bold",size="large")
bx.set_ylabel('Overall Execution Time (x10^3 in ms)')
bx.set_xlabel('Deviation from True Bids')
bx.text(1.2,0.617,'Green -- = OPTIM algorithm\nBlue -- = With false bids\n',bbox={'facecolor':'white','pad':10})

rtt=[0.618178,0.605093,0.605751,0.604008,0.607982]
rtt=[0.6024,0.6024,0.6024,0.6024,0.6024]
bx.plot(p,rtt,linestyle='--',marker='o',linewidth=1.5)

plt.savefig('graph.png')
