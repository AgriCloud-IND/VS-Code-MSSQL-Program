USE [MMC]
GO
ALTER TABLE [dbo].[TransactionMaster] DROP CONSTRAINT [FK_TransactionMaster_Users]
GO
ALTER TABLE [dbo].[TransactionMaster] DROP CONSTRAINT [FK_TransactionMaster_customers]
GO
ALTER TABLE [dbo].[TransactionDetails] DROP CONSTRAINT [FK_TransactionDetails_Vedios]
GO
ALTER TABLE [dbo].[TransactionDetails] DROP CONSTRAINT [FK_TransactionDetails_TransactionMaster]
GO
/****** Object:  Table [dbo].[Video]    Script Date: 17-02-2024 20:39:58 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Video]') AND type in (N'U'))
DROP TABLE [dbo].[Video]
GO
/****** Object:  Table [dbo].[Users]    Script Date: 17-02-2024 20:39:58 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Users]') AND type in (N'U'))
DROP TABLE [dbo].[Users]
GO
/****** Object:  Table [dbo].[TransactionMaster]    Script Date: 17-02-2024 20:39:58 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TransactionMaster]') AND type in (N'U'))
DROP TABLE [dbo].[TransactionMaster]
GO
/****** Object:  Table [dbo].[TransactionDetails]    Script Date: 17-02-2024 20:39:58 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TransactionDetails]') AND type in (N'U'))
DROP TABLE [dbo].[TransactionDetails]
GO
/****** Object:  Table [dbo].[customers]    Script Date: 17-02-2024 20:39:58 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[customers]') AND type in (N'U'))
DROP TABLE [dbo].[customers]
GO
/****** Object:  Table [dbo].[customers]    Script Date: 17-02-2024 20:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[customers](
	[customer_id] [int] IDENTITY(1,1) NOT NULL,
	[customer_name] [nvarchar](50) NULL,
	[customer_email] [nvarchar](50) NULL,
	[customer_phone] [nvarchar](50) NULL,
	[customer_address] [nvarchar](250) NULL,
 CONSTRAINT [PK_customers] PRIMARY KEY CLUSTERED 
(
	[customer_id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[TransactionDetails]    Script Date: 17-02-2024 20:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TransactionDetails](
	[TrID] [int] NOT NULL,
	[VideoID] [int] NOT NULL,
	[VideoName] [nvarchar](50) NULL,
	[Rent] [float] NULL,
 CONSTRAINT [PK_TransactionDetails] PRIMARY KEY CLUSTERED 
(
	[TrID] ASC,
	[VideoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[TransactionMaster]    Script Date: 17-02-2024 20:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TransactionMaster](
	[TrID] [int] IDENTITY(1,1) NOT NULL,
	[TrDate] [date] NULL,
	[CustomerID] [int] NULL,
	[EmployeeID] [int] NULL,
 CONSTRAINT [PK_TransactionMaster] PRIMARY KEY CLUSTERED 
(
	[TrID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Users]    Script Date: 17-02-2024 20:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Users](
	[UserID] [int] IDENTITY(1,1) NOT NULL,
	[Name] [varchar](50) NULL,
	[Email] [varchar](50) NULL,
 CONSTRAINT [PK_Users] PRIMARY KEY CLUSTERED 
(
	[UserID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[Video]    Script Date: 17-02-2024 20:39:58 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Video](
	[VideoID] [int] IDENTITY(1,1) NOT NULL,
	[VideoName] [nvarchar](50) NULL,
	[ActorName] [nvarchar](50) NULL,
	[ReleaseYear] [int] NULL,
 CONSTRAINT [PK_Vedios] PRIMARY KEY CLUSTERED 
(
	[VideoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, OPTIMIZE_FOR_SEQUENTIAL_KEY = OFF) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[TransactionDetails]  WITH CHECK ADD  CONSTRAINT [FK_TransactionDetails_TransactionMaster] FOREIGN KEY([TrID])
REFERENCES [dbo].[TransactionMaster] ([TrID])
GO
ALTER TABLE [dbo].[TransactionDetails] CHECK CONSTRAINT [FK_TransactionDetails_TransactionMaster]
GO
ALTER TABLE [dbo].[TransactionDetails]  WITH CHECK ADD  CONSTRAINT [FK_TransactionDetails_Vedios] FOREIGN KEY([VideoID])
REFERENCES [dbo].[Video] ([VideoID])
GO
ALTER TABLE [dbo].[TransactionDetails] CHECK CONSTRAINT [FK_TransactionDetails_Vedios]
GO
ALTER TABLE [dbo].[TransactionMaster]  WITH CHECK ADD  CONSTRAINT [FK_TransactionMaster_customers] FOREIGN KEY([CustomerID])
REFERENCES [dbo].[customers] ([customer_id])
GO
ALTER TABLE [dbo].[TransactionMaster] CHECK CONSTRAINT [FK_TransactionMaster_customers]
GO
ALTER TABLE [dbo].[TransactionMaster]  WITH CHECK ADD  CONSTRAINT [FK_TransactionMaster_Users] FOREIGN KEY([EmployeeID])
REFERENCES [dbo].[Users] ([UserID])
GO
ALTER TABLE [dbo].[TransactionMaster] CHECK CONSTRAINT [FK_TransactionMaster_Users]
GO
