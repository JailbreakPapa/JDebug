AssetProfiles
{
	Config %PC
	{
		Objects
		{
			o
			{
				Uuid %id{uint64{8085892115830203315,3737221074888337082}}
				string %t{"nsPlatformProfile"}
				uint32 %v{1}
				string %n{"root"}
				p
				{
					VarArray %Configs
					{
						Uuid{uint64{11754538365307859884,8572204067211736280}}
						Uuid{uint64{18165925277888737543,17003377905246685588}}
						Uuid{uint64{10688866045028468411,17021851764261564183}}
					}
					string %Name{"PC"}
					string %Platform{"nsProfileTargetPlatform::PC"}
				}
			}
			o
			{
				Uuid %id{uint64{11754538365307859884,8572204067211736280}}
				string %t{"nsRenderPipelineProfileConfig"}
				uint32 %v{1}
				p
				{
					VarDict %CameraPipelines{}
					string %MainRenderPipeline{"{ c533e113-2a4c-4f42-a546-653c78f5e8a7 }"}
				}
			}
			o
			{
				Uuid %id{uint64{18165925277888737543,17003377905246685588}}
				string %t{"nsTextureAssetProfileConfig"}
				uint32 %v{1}
				p
				{
					uint16 %MaxResolution{16384}
				}
			}
			o
			{
				Uuid %id{uint64{10688866045028468411,17021851764261564183}}
				string %t{"nsXRConfig"}
				uint32 %v{2}
				p
				{
					bool %EnableXR{false}
					string %XRRenderPipeline{"{ 2fe25ded-776c-7f9e-354f-e4c52a33d125 }"}
				}
			}
		}
	}
	Config %PS5
	{
		Objects
		{
			o
			{
				Uuid %id{uint64{5441518849640516000,407077599152189293}}
				string %t{"nsPlatformProfile"}
				uint32 %v{1}
				string %n{"root"}
				p
				{
					VarArray %Configs
					{
						Uuid{uint64{11455110905756167244,12701896497253451912}}
						Uuid{uint64{7500695947391545795,17525598327313743136}}
						Uuid{uint64{10362997729182465544,8207978427172368145}}
					}
					string %Name{"PS5"}
					string %Platform{"nsProfileTargetPlatform::PS5"}
				}
			}
			o
			{
				Uuid %id{uint64{10362997729182465544,8207978427172368145}}
				string %t{"nsXRConfig"}
				uint32 %v{2}
				p
				{
					bool %EnableXR{false}
					string %XRRenderPipeline{"{ 2fe25ded-776c-7f9e-354f-e4c52a33d125 }"}
				}
			}
			o
			{
				Uuid %id{uint64{11455110905756167244,12701896497253451912}}
				string %t{"nsRenderPipelineProfileConfig"}
				uint32 %v{1}
				p
				{
					VarDict %CameraPipelines{}
					string %MainRenderPipeline{"{ c533e113-2a4c-4f42-a546-653c78f5e8a7 }"}
				}
			}
			o
			{
				Uuid %id{uint64{7500695947391545795,17525598327313743136}}
				string %t{"nsTextureAssetProfileConfig"}
				uint32 %v{1}
				p
				{
					uint16 %MaxResolution{16384}
				}
			}
		}
	}
}
