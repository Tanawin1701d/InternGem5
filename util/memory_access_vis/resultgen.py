import matplotlib.pyplot as plt
fig = plt.figure()
method = ['fcfs', 'frfcfs-64', 'frfcfs-128', 'sms-32']
dayta = [[43.03,48.18,54.62,58.01],
            [16.61,47.45,62.34,63.62],
            ]

X = np.arrange(4)
fig = plt.figure()
ax = fig.add_axes([0,0,0,0])
ax.bar(X + 0.00, data[0], color = 'b', width = 0.25)
ax.bar(X + 0.25, data[1], color = 'g', width = 0.25)
ax.bar(X + 0.50, data[2], color = 'r', width = 0.25)

